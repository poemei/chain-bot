#ifndef CHAIN_BOT_SLACK_H
#define CHAIN_BOT_SLACK_H

#include <stddef.h>

#include "chain_bot_config.h"

int chain_bot_slack_announce(
    const chain_bot_slack_config_t *config,
    const char *message,
    char *error,
    size_t error_size);

#endif
