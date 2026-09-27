#include <stdio.h>
#include <string.h>

#include "chain_bot_irc_commands.h"

static int test_help(void)
{
    char response[CHAIN_BOT_IRC_COMMAND_RESPONSE_MAX];
    char error[64];

    return chain_bot_irc_command_handle("!help", NULL, response, sizeof(response), error, sizeof(error)) &&
           strstr(response, "!status") != NULL && strstr(response, "!height") != NULL;
}

static int test_about(void)
{
    char response[CHAIN_BOT_IRC_COMMAND_RESPONSE_MAX];
    char error[64];

    return chain_bot_irc_command_handle("!about", NULL, response, sizeof(response), error, sizeof(error)) &&
           strstr(response, "deterministic consensus network") != NULL;
}

static int test_whitepaper(void)
{
    char response[CHAIN_BOT_IRC_COMMAND_RESPONSE_MAX];
    char error[64];

    return chain_bot_irc_command_handle("!whitepaper", NULL, response, sizeof(response), error, sizeof(error)) &&
           strstr(response, "stn-chain.org") != NULL;
}

static int test_website(void)
{
    char response[CHAIN_BOT_IRC_COMMAND_RESPONSE_MAX];
    char error[64];

    return chain_bot_irc_command_handle("!website", NULL, response, sizeof(response), error, sizeof(error)) &&
           strcmp(response, "STN Chain: https://stn-chain.org") == 0;
}

static int test_bot(void)
{
    char response[CHAIN_BOT_IRC_COMMAND_RESPONSE_MAX];
    char error[64];

    return chain_bot_irc_command_handle("!bot", NULL, response, sizeof(response), error, sizeof(error)) &&
           strstr(response, "Linux ISO C") != NULL;
}

static int test_unknown_is_ignored(void)
{
    char response[CHAIN_BOT_IRC_COMMAND_RESPONSE_MAX];
    char error[64];

    return !chain_bot_irc_command_handle("!dance", NULL, response, sizeof(response), error, sizeof(error)) &&
           response[0] == '\0';
}

static int test_non_command_is_ignored(void)
{
    char response[CHAIN_BOT_IRC_COMMAND_RESPONSE_MAX];
    char error[64];

    return !chain_bot_irc_command_handle("hello", NULL, response, sizeof(response), error, sizeof(error)) &&
           response[0] == '\0';
}

static int test_chain_command_without_config_is_bounded(void)
{
    char response[CHAIN_BOT_IRC_COMMAND_RESPONSE_MAX];
    char error[64];

    return chain_bot_irc_command_handle("!height", NULL, response, sizeof(response), error, sizeof(error)) &&
           strcmp(response, "STN Chain information is temporarily unavailable.") == 0;
}

static int test_invalid_arguments(void)
{
    char response[CHAIN_BOT_IRC_COMMAND_RESPONSE_MAX];
    char error[64];

    return !chain_bot_irc_command_handle(NULL, NULL, response, sizeof(response), error, sizeof(error)) &&
           !chain_bot_irc_command_handle("!help", NULL, NULL, 0U, error, sizeof(error));
}

int main(void)
{
    unsigned int executed = 0U;
    unsigned int passed = 0U;

#define RUN_TEST(test_fn) \
    do { ++executed; if ((test_fn)()) { ++passed; printf("[PASS] %s\n", #test_fn); } \
         else { printf("[FAIL] %s\n", #test_fn); } } while (0)

    RUN_TEST(test_help);
    RUN_TEST(test_about);
    RUN_TEST(test_whitepaper);
    RUN_TEST(test_website);
    RUN_TEST(test_bot);
    RUN_TEST(test_unknown_is_ignored);
    RUN_TEST(test_non_command_is_ignored);
    RUN_TEST(test_chain_command_without_config_is_bounded);
    RUN_TEST(test_invalid_arguments);

#undef RUN_TEST

    printf("Chain Bot public IRC command tests: %u/%u passed.\n", passed, executed);
    return passed == executed ? 0 : 1;
}
