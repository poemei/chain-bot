#ifndef CHAIN_BOT_LOG_H
#define CHAIN_BOT_LOG_H

#include <stddef.h>

#define CHAIN_BOT_LOG_DEFAULT_PATH "/opt/chain-bot/logs/chain-bot.log"

int chain_bot_log_open(const char *path, char *error, size_t error_size);
void chain_bot_log_info(const char *message);
void chain_bot_log_error(const char *message);
void chain_bot_log_close(void);

#endif
