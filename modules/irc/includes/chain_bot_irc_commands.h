#ifndef CHAIN_BOT_IRC_COMMANDS_H
#define CHAIN_BOT_IRC_COMMANDS_H

#include <stddef.h>

#include "chain_bot_config.h"

#define CHAIN_BOT_IRC_COMMAND_RESPONSE_MAX 480U

int chain_bot_irc_command_handle(
    const char *message,
    const chain_bot_chain_config_t *chain_config,
    char *response,
    size_t response_size,
    char *error,
    size_t error_size);

#endif
