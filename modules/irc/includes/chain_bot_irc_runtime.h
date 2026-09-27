#ifndef CHAIN_BOT_IRC_RUNTIME_H
#define CHAIN_BOT_IRC_RUNTIME_H

#include <stddef.h>

#include "chain_bot_config.h"
#include "chain_bot_irc_session.h"

typedef struct chain_bot_irc_runtime {
    chain_bot_irc_transport_t transport;
    chain_bot_irc_session_t session;
    int connected;
} chain_bot_irc_runtime_t;

void chain_bot_irc_runtime_init(chain_bot_irc_runtime_t *runtime);

int chain_bot_irc_runtime_connect(
    chain_bot_irc_runtime_t *runtime,
    const chain_bot_irc_config_t *config,
    char *error,
    size_t error_size);

int chain_bot_irc_runtime_receive(
    chain_bot_irc_runtime_t *runtime,
    char *buffer,
    size_t buffer_size,
    size_t *received,
    char *error,
    size_t error_size);

int chain_bot_irc_runtime_handle_line(
    chain_bot_irc_runtime_t *runtime,
    const char *line,
    char *error,
    size_t error_size);

int chain_bot_irc_runtime_announce(
    chain_bot_irc_runtime_t *runtime,
    const char *message,
    char *error,
    size_t error_size);

void chain_bot_irc_runtime_close(chain_bot_irc_runtime_t *runtime);

#endif
