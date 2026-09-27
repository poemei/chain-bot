#include <stdio.h>
#include <string.h>

#include "chain_bot_irc_worker.h"

static int test_init(void)
{
    chain_bot_irc_runtime_t runtime;
    chain_bot_irc_worker_t worker;

    chain_bot_irc_runtime_init(&runtime);
    memset(&worker, 0xff, sizeof(worker));
    chain_bot_irc_worker_init(&worker, &runtime);

    return worker.loop.runtime == &runtime && !worker.running &&
           !worker.stop_requested && !worker.failed;
}

static int test_start_rejects_null(void)
{
    return !chain_bot_irc_worker_start(NULL);
}

static int test_start_requires_runtime(void)
{
    chain_bot_irc_worker_t worker;

    chain_bot_irc_worker_init(&worker, NULL);
    return !chain_bot_irc_worker_start(&worker);
}

static int test_start_requires_connection(void)
{
    chain_bot_irc_runtime_t runtime;
    chain_bot_irc_worker_t worker;

    chain_bot_irc_runtime_init(&runtime);
    chain_bot_irc_worker_init(&worker, &runtime);

    return !chain_bot_irc_worker_start(&worker);
}

static int test_stop_idle_worker(void)
{
    chain_bot_irc_runtime_t runtime;
    chain_bot_irc_worker_t worker;

    chain_bot_irc_runtime_init(&runtime);
    chain_bot_irc_worker_init(&worker, &runtime);
    chain_bot_irc_worker_stop(&worker);

    return !worker.running && worker.stop_requested;
}

static int test_failed_null_is_safe(void)
{
    return chain_bot_irc_worker_failed(NULL) == 1;
}

static int test_failed_initial_state(void)
{
    chain_bot_irc_runtime_t runtime;
    chain_bot_irc_worker_t worker;

    chain_bot_irc_runtime_init(&runtime);
    chain_bot_irc_worker_init(&worker, &runtime);

    return chain_bot_irc_worker_failed(&worker) == 0;
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
    RUN_TEST(test_start_rejects_null);
    RUN_TEST(test_start_requires_runtime);
    RUN_TEST(test_start_requires_connection);
    RUN_TEST(test_stop_idle_worker);
    RUN_TEST(test_failed_null_is_safe);
    RUN_TEST(test_failed_initial_state);

#undef RUN_TEST

    printf("Chain Bot IRC worker tests: %u/%u passed.\n", passed, executed);
    return passed == executed ? 0 : 1;
}
