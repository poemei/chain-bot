#include <stdio.h>
#include <string.h>

#include "chain_bot_module.h"

static int test_init(void)
{
    chain_bot_module_t module;

    memset(&module, 0xff, sizeof(module));
    chain_bot_module_init(&module);

    return module.handle == NULL && module.descriptor == NULL &&
           module.path[0] == '\0' && module.active == 0;
}

static int test_invalid_load_arguments(void)
{
    chain_bot_module_t module;
    stnlabz_module_registry_t registry;

    chain_bot_module_init(&module);
    stnlabz_module_registry_init(&registry);

    return chain_bot_module_load(NULL, "module.so", &registry) ==
               CHAIN_BOT_MODULE_ERR_INVALID_ARGUMENT &&
           chain_bot_module_load(&module, NULL, &registry) ==
               CHAIN_BOT_MODULE_ERR_INVALID_ARGUMENT &&
           chain_bot_module_load(&module, "", &registry) ==
               CHAIN_BOT_MODULE_ERR_INVALID_ARGUMENT &&
           chain_bot_module_load(&module, "module.so", NULL) ==
               CHAIN_BOT_MODULE_ERR_INVALID_ARGUMENT;
}

static int test_missing_module(void)
{
    chain_bot_module_t module;
    stnlabz_module_registry_t registry;

    chain_bot_module_init(&module);
    stnlabz_module_registry_init(&registry);

    return chain_bot_module_load(
               &module,
               "./does-not-exist-chain-bot-module.so",
               &registry) == CHAIN_BOT_MODULE_ERR_LOAD_FAILED &&
           module.handle == NULL && module.descriptor == NULL;
}

static int test_invalid_activate_arguments(void)
{
    chain_bot_module_t module;
    stnlabz_module_registry_t registry;
    stnlabz_module_host_t host;

    chain_bot_module_init(&module);
    stnlabz_module_registry_init(&registry);
    memset(&host, 0, sizeof(host));

    return chain_bot_module_activate(NULL, &registry, &host) ==
               CHAIN_BOT_MODULE_ERR_INVALID_ARGUMENT &&
           chain_bot_module_activate(&module, NULL, &host) ==
               CHAIN_BOT_MODULE_ERR_INVALID_ARGUMENT &&
           chain_bot_module_activate(&module, &registry, &host) ==
               CHAIN_BOT_MODULE_ERR_INVALID_ARGUMENT;
}

static int test_empty_unload(void)
{
    chain_bot_module_t module;
    stnlabz_module_registry_t registry;

    chain_bot_module_init(&module);
    stnlabz_module_registry_init(&registry);

    return chain_bot_module_unload(&module, &registry) == CHAIN_BOT_MODULE_OK &&
           module.handle == NULL && module.active == 0;
}

static int test_result_strings(void)
{
    return strcmp(chain_bot_module_result_string(CHAIN_BOT_MODULE_OK), "ok") == 0 &&
           strcmp(chain_bot_module_result_string(CHAIN_BOT_MODULE_ERR_VERIFY_FAILED),
                  "verification failed") == 0 &&
           strcmp(chain_bot_module_result_string(CHAIN_BOT_MODULE_ERR_QUALIFY_FAILED),
                  "qualification failed") == 0 &&
           strcmp(chain_bot_module_result_string(
                      CHAIN_BOT_MODULE_ERR_QUALIFICATION_GATE_FAILED),
                  "qualification gate failed") == 0 &&
           strcmp(chain_bot_module_result_string(CHAIN_BOT_MODULE_ERR_AUTHORIZE_FAILED),
                  "activation authorization failed") == 0 &&
           strcmp(chain_bot_module_result_string(CHAIN_BOT_MODULE_ERR_START_FAILED),
                  "module start failed") == 0;
}

int main(void)
{
    unsigned int executed = 0U;
    unsigned int passed = 0U;

#define RUN_TEST(test_fn) \
    do { \
        ++executed; \
        if ((test_fn)()) { ++passed; printf("[PASS] %s\n", #test_fn); } \
        else { printf("[FAIL] %s\n", #test_fn); } \
    } while (0)

    RUN_TEST(test_init);
    RUN_TEST(test_invalid_load_arguments);
    RUN_TEST(test_missing_module);
    RUN_TEST(test_invalid_activate_arguments);
    RUN_TEST(test_empty_unload);
    RUN_TEST(test_result_strings);

#undef RUN_TEST

    printf("Chain Bot module host tests: %u/%u passed.\n", passed, executed);
    return passed == executed ? 0 : 1;
}
