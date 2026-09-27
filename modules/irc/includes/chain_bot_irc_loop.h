#ifndef CHAIN_BOT_IRC_LOOP_H
#define CHAIN_BOT_IRC_LOOP_H

#include <stddef.h>

#include "chain_bot_config.h"
#include "chain_bot_irc_runtime.h"

#define CHAIN_BOT_IRC_LOOP_BUFFER_MAX 2048

typedef struct chain_bot_irc_loop {
    chain_bot_irc_runtime_t *runtime;
    const chain_bot_chain_config_t *chain_config;
    char pending[CHAIN_BOT_IRC_LOOP_BUFFER_MAX];
    size_t pending_length;
} chain_bot_irc_loop_t;

void chain_bot_irc_loop_init(
    chain_bot_irc_loop_t *loop,
    chain_bot_irc_runtime_t *runtime);

void chain_bot_irc_loop_set_chain_config(
    chain_bot_irc_loop_t *loop,
    const chain_bot_chain_config_t *chain_config);

int chain_bot_irc_loop_feed(
    chain_bot_irc_loop_t *loop,
    const char *data,
    size_t length,
    char *error,
    size_t error_size);

int chain_bot_irc_loop_receive_once(
    chain_bot_irc_loop_t *loop,
    char *error,
    size_t error_size);

#endif
