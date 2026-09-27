#ifndef CHAIN_BOT_H
#define CHAIN_BOT_H

#define CHAIN_BOT_NAME "Chain Bot"
#define CHAIN_BOT_VERSION_MAJOR 0
#define CHAIN_BOT_VERSION_MINOR 1
#define CHAIN_BOT_VERSION_PATCH 0

typedef enum chain_bot_result {
    CHAIN_BOT_OK = 0,
    CHAIN_BOT_ERR_INVALID_ARGUMENT = 1
} chain_bot_result_t;

typedef struct chain_bot {
    int initialized;
} chain_bot_t;

chain_bot_result_t chain_bot_init(chain_bot_t *bot);
void chain_bot_shutdown(chain_bot_t *bot);
const char *chain_bot_name(void);

#endif
