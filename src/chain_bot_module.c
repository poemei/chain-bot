#include <dlfcn.h>
#include <string.h>

#include "chain_bot_module.h"

void chain_bot_module_init(chain_bot_module_t *module)
{
    if (module == NULL) {
        return;
    }

    memset(module, 0, sizeof(*module));
}

chain_bot_module_result_t chain_bot_module_load(
    chain_bot_module_t *module,
    const char *path,
    stnlabz_module_registry_t *registry)
{
    chain_bot_module_get_descriptor_fn get_descriptor;
    const stnlabz_module_descriptor_t *descriptor;
    stnlabz_module_result_t abi_result;
    size_t path_length;

    if (module == NULL || path == NULL || path[0] == '\0' || registry == NULL) {
        return CHAIN_BOT_MODULE_ERR_INVALID_ARGUMENT;
    }

    if (module->handle != NULL) {
        return CHAIN_BOT_MODULE_ERR_INVALID_ARGUMENT;
    }

    path_length = strlen(path);
    if (path_length >= sizeof(module->path)) {
        return CHAIN_BOT_MODULE_ERR_INVALID_ARGUMENT;
    }

    module->handle = dlopen(path, RTLD_NOW | RTLD_LOCAL);
    if (module->handle == NULL) {
        return CHAIN_BOT_MODULE_ERR_LOAD_FAILED;
    }

    dlerror();
    *(void **)(&get_descriptor) = dlsym(
        module->handle,
        CHAIN_BOT_MODULE_DESCRIPTOR_EXPORT);

    if (get_descriptor == NULL || dlerror() != NULL) {
        dlclose(module->handle);
        chain_bot_module_init(module);
        return CHAIN_BOT_MODULE_ERR_EXPORT_MISSING;
    }

    descriptor = get_descriptor();
    if (descriptor == NULL || descriptor->id[0] == '\0' ||
        descriptor->name[0] == '\0' || descriptor->qualify == NULL) {
        dlclose(module->handle);
        chain_bot_module_init(module);
        return CHAIN_BOT_MODULE_ERR_DESCRIPTOR_INVALID;
    }

    abi_result = stnlabz_module_registry_register(registry, descriptor);
    if (abi_result != STNLABZ_MODULE_OK) {
        dlclose(module->handle);
        chain_bot_module_init(module);
        return CHAIN_BOT_MODULE_ERR_VERIFY_FAILED;
    }

    abi_result = stnlabz_module_registry_verify(registry, descriptor->id);
    if (abi_result != STNLABZ_MODULE_OK) {
        (void)stnlabz_module_abi_unregister(registry, descriptor->id);
        dlclose(module->handle);
        chain_bot_module_init(module);
        return CHAIN_BOT_MODULE_ERR_VERIFY_FAILED;
    }

    abi_result = stnlabz_module_registry_qualify(registry, descriptor->id);
    if (abi_result != STNLABZ_MODULE_OK) {
        (void)stnlabz_module_abi_unregister(registry, descriptor->id);
        dlclose(module->handle);
        chain_bot_module_init(module);
        return CHAIN_BOT_MODULE_ERR_QUALIFY_FAILED;
    }

    {
        stnlabz_module_record_t *record =
            stnlabz_module_registry_find(registry, descriptor->id);

        if (record == NULL ||
            record->state != STNLABZ_MODULE_STATE_QUALIFIED ||
            record->qualification.tests_executed < STNLABZ_MODULE_MIN_TESTS ||
            record->qualification.tests_passed != record->qualification.tests_executed ||
            record->qualification.tests_failed != 0U ||
            !record->qualification.negative_test_executed ||
            !record->qualification.negative_test_passed) {
            (void)stnlabz_module_abi_unregister(registry, descriptor->id);
            dlclose(module->handle);
            chain_bot_module_init(module);
            return CHAIN_BOT_MODULE_ERR_QUALIFICATION_GATE_FAILED;
        }
    }

    module->descriptor = descriptor;
    memcpy(module->path, path, path_length + 1U);
    module->active = 0;

    return CHAIN_BOT_MODULE_OK;
}

chain_bot_module_result_t chain_bot_module_activate(
    chain_bot_module_t *module,
    stnlabz_module_registry_t *registry,
    const stnlabz_module_host_t *host)
{
    stnlabz_module_result_t result;

    if (module == NULL || registry == NULL || module->handle == NULL ||
        module->descriptor == NULL || module->active) {
        return CHAIN_BOT_MODULE_ERR_INVALID_ARGUMENT;
    }

    result = stnlabz_module_registry_authorize_activation(
        registry,
        module->descriptor->id);
    if (result != STNLABZ_MODULE_OK) {
        return CHAIN_BOT_MODULE_ERR_AUTHORIZE_FAILED;
    }

    result = stnlabz_module_registry_activate(
        registry,
        module->descriptor->id);
    if (result != STNLABZ_MODULE_OK) {
        return CHAIN_BOT_MODULE_ERR_ACTIVATE_FAILED;
    }

    if (module->descriptor->start != NULL &&
        module->descriptor->start(host) != STNLABZ_MODULE_OK) {
        (void)stnlabz_module_registry_fail(registry, module->descriptor->id);
        return CHAIN_BOT_MODULE_ERR_START_FAILED;
    }

    module->active = 1;
    return CHAIN_BOT_MODULE_OK;
}

chain_bot_module_result_t chain_bot_module_unload(
    chain_bot_module_t *module,
    stnlabz_module_registry_t *registry)
{
    stnlabz_module_result_t result;

    if (module == NULL || registry == NULL) {
        return CHAIN_BOT_MODULE_ERR_INVALID_ARGUMENT;
    }

    if (module->handle == NULL) {
        chain_bot_module_init(module);
        return CHAIN_BOT_MODULE_OK;
    }

    if (module->active) {
        result = stnlabz_module_abi_stop(registry, module->descriptor->id);
        if (result != STNLABZ_MODULE_OK) {
            return CHAIN_BOT_MODULE_ERR_STOP_FAILED;
        }
        module->active = 0;
    }

    result = stnlabz_module_abi_unregister(registry, module->descriptor->id);
    if (result != STNLABZ_MODULE_OK) {
        return CHAIN_BOT_MODULE_ERR_STOP_FAILED;
    }

    (void)dlclose(module->handle);
    chain_bot_module_init(module);
    return CHAIN_BOT_MODULE_OK;
}

const char *chain_bot_module_result_string(chain_bot_module_result_t result)
{
    switch (result) {
        case CHAIN_BOT_MODULE_OK: return "ok";
        case CHAIN_BOT_MODULE_ERR_INVALID_ARGUMENT: return "invalid argument";
        case CHAIN_BOT_MODULE_ERR_LOAD_FAILED: return "load failed";
        case CHAIN_BOT_MODULE_ERR_EXPORT_MISSING: return "descriptor export missing";
        case CHAIN_BOT_MODULE_ERR_DESCRIPTOR_INVALID: return "descriptor invalid";
        case CHAIN_BOT_MODULE_ERR_VERIFY_FAILED: return "verification failed";
        case CHAIN_BOT_MODULE_ERR_QUALIFY_FAILED: return "qualification failed";
        case CHAIN_BOT_MODULE_ERR_QUALIFICATION_GATE_FAILED: return "qualification gate failed";
        case CHAIN_BOT_MODULE_ERR_AUTHORIZE_FAILED: return "activation authorization failed";
        case CHAIN_BOT_MODULE_ERR_ACTIVATE_FAILED: return "activation failed";
        case CHAIN_BOT_MODULE_ERR_START_FAILED: return "module start failed";
        case CHAIN_BOT_MODULE_ERR_STOP_FAILED: return "stop or unregister failed";
        default: return "unknown";
    }
}
