#ifndef CHAIN_BOT_IRC_RUNTIME_H
#define CHAIN_BOT_IRC_RUNTIME_H

#include "../../../../includes/chain_bot_config.h"
#include "chain_bot_irc_session.h"

typedef struct chain_bot_irc_runtime {
    chain_bot_irc_transport_t transport;
    chain_bot_irc_session_t session;
    int started;
} chain_bot_irc_runtime_t;

void chain_bot_irc_runtime_init(chain_bot_irc_runtime_t *runtime);

int chain_bot_irc_runtime_start(
    chain_bot_irc_runtime_t *runtime,
    const chain_bot_irc_config_t *config,
    char *error,
    size_t error_size);

int chain_bot_irc_runtime_process(
    chain_bot_irc_runtime_t *runtime,
    char *error,
    size_t error_size);

void chain_bot_irc_runtime_stop(chain_bot_irc_runtime_t *runtime);

#endif
