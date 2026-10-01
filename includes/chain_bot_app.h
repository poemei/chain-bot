#ifndef CHAIN_BOT_APP_H
#define CHAIN_BOT_APP_H

#include "chain_bot_config.h"
#include "chain_bot_irc_runtime.h"
#include "chain_bot_irc_worker.h"
#include "chain_bot_observer.h"
#include "chain_bot_slack.h"

typedef struct chain_bot_app {
    chain_bot_config_t config;
    chain_bot_irc_runtime_t irc_runtime;
    chain_bot_irc_worker_t irc_worker;
    chain_bot_observer_t observer;
    int running;
} chain_bot_app_t;

void chain_bot_app_init(chain_bot_app_t *app);
int chain_bot_app_start(chain_bot_app_t *app,const char *config_path,char *error,size_t error_size);
int chain_bot_app_run(chain_bot_app_t *app);
void chain_bot_app_stop(chain_bot_app_t *app);

#endif
