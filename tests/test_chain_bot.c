#include <stdio.h>
#include <string.h>

#include "chain_bot.h"

static int test_init(void)
{
    chain_bot_t bot = {0};

    if (chain_bot_init(&bot) != CHAIN_BOT_OK) {
        return 0;
    }

    return bot.initialized == 1;
}

static int test_shutdown(void)
{
    chain_bot_t bot = {0};

    if (chain_bot_init(&bot) != CHAIN_BOT_OK) {
        return 0;
    }

    chain_bot_shutdown(&bot);
    return bot.initialized == 0;
}

static int test_name(void)
{
    return strcmp(chain_bot_name(), "Chain Bot") == 0;
}

static int test_invalid_init(void)
{
    return chain_bot_init(NULL) == CHAIN_BOT_ERR_INVALID_ARGUMENT;
}

int main(void)
{
    unsigned int executed = 0U;
    unsigned int passed = 0U;

#define RUN_TEST(test_fn) \
    do { \
        ++executed; \
        if ((test_fn)()) { \
            ++passed; \
            printf("[PASS] %s\n", #test_fn); \
        } else { \
            printf("[FAIL] %s\n", #test_fn); \
        } \
    } while (0)

    RUN_TEST(test_init);
    RUN_TEST(test_shutdown);
    RUN_TEST(test_name);
    RUN_TEST(test_invalid_init);

#undef RUN_TEST

    printf("Chain Bot bootstrap tests: %u/%u passed.\n", passed, executed);
    return passed == executed ? 0 : 1;
}
