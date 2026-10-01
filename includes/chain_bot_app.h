#ifndef CHAIN_BOT_APP_H
#define CHAIN_BOT_APP_H

#include "chain_bot_config.h"
#include "chain_bot_irc_runtime.h"
#include "chain_bot_irc_worker.h"
#include "chain_bot_module.h"
#include "chain_bot_observer.h"

#define CHAIN_BOT_IRC_MODULE_PATH "/opt/chain-bot/modules/chain_bot_irc.so"
#define CHAIN_BOT_SLACK_MODULE_PATH "/opt/chain-bot/modules/chain_bot_slack.so"

typedef struct chain_bot_app {
    chain_bot_config_t config;
    chain_bot_irc_runtime_t irc_runtime;
    chain_bot_irc_worker_t irc_worker;
    chain_bot_observer_t observer;
    stnlabz_module_registry_t module_registry;
    stnlabz_module_host_t module_host;
    chain_bot_module_t irc_module;
    chain_bot_module_t slack_module;
    unsigned int modules_active;
    int running;
} chain_bot_app_t;

void chain_bot_app_init(chain_bot_app_t *app);
int chain_bot_app_start(chain_bot_app_t *app, const char *config_path, char *error, size_t error_size);
int chain_bot_app_run(chain_bot_app_t *app);
void chain_bot_app_stop(chain_bot_app_t *app);

#endif
