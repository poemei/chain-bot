#ifndef CHAIN_BOT_MODULE_H
#define CHAIN_BOT_MODULE_H

#include <stddef.h>

#include "../../ABI/includes/abi.h"

#define CHAIN_BOT_MODULE_PATH_MAX 1024
#define CHAIN_BOT_MODULE_DESCRIPTOR_EXPORT "stnlabz_module_get_descriptor"

typedef enum chain_bot_module_result {
    CHAIN_BOT_MODULE_OK = 0,
    CHAIN_BOT_MODULE_ERR_INVALID_ARGUMENT,
    CHAIN_BOT_MODULE_ERR_LOAD_FAILED,
    CHAIN_BOT_MODULE_ERR_EXPORT_MISSING,
    CHAIN_BOT_MODULE_ERR_DESCRIPTOR_INVALID,
    CHAIN_BOT_MODULE_ERR_PREPARE_FAILED,
    CHAIN_BOT_MODULE_ERR_ACTIVATE_FAILED,
    CHAIN_BOT_MODULE_ERR_STOP_FAILED
} chain_bot_module_result_t;

typedef const stnlabz_module_descriptor_t *
(*chain_bot_module_get_descriptor_fn)(void);

typedef struct chain_bot_module {
    void *handle;
    const stnlabz_module_descriptor_t *descriptor;
    char path[CHAIN_BOT_MODULE_PATH_MAX];
    int active;
} chain_bot_module_t;

void chain_bot_module_init(chain_bot_module_t *module);

chain_bot_module_result_t chain_bot_module_load(
    chain_bot_module_t *module,
    const char *path,
    stnlabz_module_registry_t *registry);

chain_bot_module_result_t chain_bot_module_activate(
    chain_bot_module_t *module,
    stnlabz_module_registry_t *registry,
    const stnlabz_module_host_t *host);

chain_bot_module_result_t chain_bot_module_unload(
    chain_bot_module_t *module,
    stnlabz_module_registry_t *registry);

const char *chain_bot_module_result_string(chain_bot_module_result_t result);

#endif
