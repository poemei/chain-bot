#ifndef CHAIN_BOT_SLACK_MODULE_H
#define CHAIN_BOT_SLACK_MODULE_H

#include "../../../../ABI/includes/module.h"

#define CHAIN_BOT_SLACK_MODULE_ID "slack"
#define CHAIN_BOT_SLACK_MODULE_NAME "Chain Bot Slack"
#define CHAIN_BOT_SLACK_MODULE_VERSION_MAJOR 1
#define CHAIN_BOT_SLACK_MODULE_VERSION_MINOR 0
#define CHAIN_BOT_SLACK_MODULE_VERSION_PATCH 0

const stnlabz_module_descriptor_t *chain_bot_slack_module_descriptor(void);
const stnlabz_module_descriptor_t *stnlabz_module_get_descriptor(void);

#endif
