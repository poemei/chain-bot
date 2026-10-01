#include <stdio.h>
#include <string.h>

#include "chain_bot_app.h"

static int test_init(void)
{
    chain_bot_app_t app;

    memset(&app, 0xff, sizeof(app));
    chain_bot_app_init(&app);

    return app.running == 0 &&
           app.modules_active == 0U &&
           app.irc_module.handle == NULL &&
           app.slack_module.handle == NULL &&
           app.irc_runtime.connected == 0 &&
           app.irc_runtime.transport.socket_fd == -1 &&
           app.irc_worker.running == 0 &&
           app.irc_worker.loop.runtime == &app.irc_runtime;
}

static int test_start_rejects_null_app(void)
{
    char error[256];

    return !chain_bot_app_start(
        NULL,
        CHAIN_BOT_CONFIG_DEFAULT_PATH,
        error,
        sizeof(error));
}

static int test_start_rejects_null_path(void)
{
    chain_bot_app_t app;
    char error[256];

    chain_bot_app_init(&app);
    return !chain_bot_app_start(&app, NULL, error, sizeof(error));
}

static int test_start_rejects_empty_path(void)
{
    chain_bot_app_t app;
    char error[256];

    chain_bot_app_init(&app);
    return !chain_bot_app_start(&app, "", error, sizeof(error));
}

static int test_run_requires_running_app(void)
{
    chain_bot_app_t app;

    chain_bot_app_init(&app);
    return !chain_bot_app_run(&app);
}

static int test_stop_is_idempotent(void)
{
    chain_bot_app_t app;

    chain_bot_app_init(&app);
    chain_bot_app_stop(&app);
    chain_bot_app_stop(&app);

    return app.running == 0 &&
           app.modules_active == 0U &&
           app.irc_module.handle == NULL &&
           app.slack_module.handle == NULL &&
           app.irc_runtime.connected == 0;
}

int main(void)
{
    unsigned int executed = 0U;
    unsigned int passed = 0U;

#define RUN_TEST(test_fn) \
    do { \
        ++executed; \
        if ((test_fn)()) { ++passed; printf("[PASS] %s\n", #test_fn); } \
        else { printf("[FAIL] %s\n", #test_fn); } \
    } while (0)

    RUN_TEST(test_init);
    RUN_TEST(test_start_rejects_null_app);
    RUN_TEST(test_start_rejects_null_path);
    RUN_TEST(test_start_rejects_empty_path);
    RUN_TEST(test_run_requires_running_app);
    RUN_TEST(test_stop_is_idempotent);

#undef RUN_TEST

    printf("Chain Bot application tests: %u/%u passed.\n", passed, executed);
    return passed == executed ? 0 : 1;
}
