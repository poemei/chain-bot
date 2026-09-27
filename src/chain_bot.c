#include <stddef.h>

#include "chain_bot.h"

chain_bot_result_t chain_bot_init(chain_bot_t *bot)
{
    if (bot == NULL) {
        return CHAIN_BOT_ERR_INVALID_ARGUMENT;
    }

    bot->initialized = 1;
    return CHAIN_BOT_OK;
}

void chain_bot_shutdown(chain_bot_t *bot)
{
    if (bot == NULL) {
        return;
    }

    bot->initialized = 0;
}

const char *chain_bot_name(void)
{
    return CHAIN_BOT_NAME;
}
