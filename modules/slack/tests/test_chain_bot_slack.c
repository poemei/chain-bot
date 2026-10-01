#include <stdio.h>
#include <string.h>
#include "chain_bot_slack.h"

static int test_disabled_is_noop(void){chain_bot_slack_config_t c;char e[128];memset(&c,0,sizeof(c));return chain_bot_slack_announce(&c,"block",e,sizeof(e));}
static int test_null_config_rejected(void){char e[128];return !chain_bot_slack_announce(NULL,"block",e,sizeof(e));}
static int test_null_message_rejected(void){chain_bot_slack_config_t c;char e[128];memset(&c,0,sizeof(c));return !chain_bot_slack_announce(&c,NULL,e,sizeof(e));}
static int test_empty_message_rejected(void){chain_bot_slack_config_t c;char e[128];memset(&c,0,sizeof(c));return !chain_bot_slack_announce(&c,"",e,sizeof(e));}
static int test_enabled_requires_https(void){chain_bot_slack_config_t c;char e[128];memset(&c,0,sizeof(c));c.enabled=1;(void)snprintf(c.webhook,sizeof(c.webhook),"http://example.invalid/hook");return !chain_bot_slack_announce(&c,"block",e,sizeof(e));}

int main(void){unsigned int n=0U,p=0U;
#define RUN(x) do{++n;if((x)()){++p;printf("[PASS] %s\n",#x);}else printf("[FAIL] %s\n",#x);}while(0)
RUN(test_disabled_is_noop);RUN(test_null_config_rejected);RUN(test_null_message_rejected);RUN(test_empty_message_rejected);RUN(test_enabled_requires_https);
#undef RUN
printf("Chain Bot Slack tests: %u/%u passed.\n",p,n);return p==n?0:1;}
