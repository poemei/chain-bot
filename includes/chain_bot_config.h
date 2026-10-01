#ifndef CHAIN_BOT_CONFIG_H
#define CHAIN_BOT_CONFIG_H

#include <stddef.h>

#define CHAIN_BOT_CONFIG_DEFAULT_PATH "config/chain_bot.json"
#define CHAIN_BOT_CONFIG_HOST_MAX 256
#define CHAIN_BOT_CONFIG_CHANNEL_MAX 128
#define CHAIN_BOT_CONFIG_NICK_MAX 64
#define CHAIN_BOT_CONFIG_PASSWORD_MAX 256
#define CHAIN_BOT_CONFIG_WEBHOOK_MAX 1024

typedef struct chain_bot_chain_config { char host[CHAIN_BOT_CONFIG_HOST_MAX]; unsigned short port; } chain_bot_chain_config_t;
typedef struct chain_bot_irc_config { char host[CHAIN_BOT_CONFIG_HOST_MAX]; unsigned short port; int tls; char channel[CHAIN_BOT_CONFIG_CHANNEL_MAX]; char nick[CHAIN_BOT_CONFIG_NICK_MAX]; char password[CHAIN_BOT_CONFIG_PASSWORD_MAX]; } chain_bot_irc_config_t;
typedef struct chain_bot_slack_config { int enabled; char webhook[CHAIN_BOT_CONFIG_WEBHOOK_MAX]; } chain_bot_slack_config_t;
typedef struct chain_bot_config { chain_bot_chain_config_t chain; chain_bot_irc_config_t irc; chain_bot_slack_config_t slack; } chain_bot_config_t;

int chain_bot_config_load(const char *path,chain_bot_config_t *config,char *error,size_t error_size);
#endif
