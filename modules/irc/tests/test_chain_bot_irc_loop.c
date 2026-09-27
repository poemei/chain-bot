#include <stdio.h>
#include <string.h>

#include "chain_bot_irc_loop.h"

static int test_init(void)
{
    chain_bot_irc_runtime_t runtime;
    chain_bot_irc_loop_t loop;

    chain_bot_irc_runtime_init(&runtime);
    memset(&loop, 0xff, sizeof(loop));
    chain_bot_irc_loop_init(&loop, &runtime);

    return loop.runtime == &runtime && loop.pending_length == 0U;
}

static int test_feed_rejects_invalid_arguments(void)
{
    chain_bot_irc_runtime_t runtime;
    chain_bot_irc_loop_t loop;
    char error[256];

    chain_bot_irc_runtime_init(&runtime);
    chain_bot_irc_loop_init(&loop, &runtime);

    return !chain_bot_irc_loop_feed(NULL, "x", 1U, error, sizeof(error)) &&
           !chain_bot_irc_loop_feed(&loop, NULL, 1U, error, sizeof(error));
}

static int test_partial_frame_is_preserved(void)
{
    chain_bot_irc_runtime_t runtime;
    chain_bot_irc_loop_t loop;
    char error[256];
    const char part[] = "PING :libera.chat\r";

    chain_bot_irc_runtime_init(&runtime);
    runtime.connected = 1;
    chain_bot_irc_loop_init(&loop, &runtime);

    return chain_bot_irc_loop_feed(
               &loop, part, sizeof(part) - 1U, error, sizeof(error)) &&
           loop.pending_length == sizeof(part) - 1U;
}

static int test_bare_lf_is_rejected(void)
{
    chain_bot_irc_runtime_t runtime;
    chain_bot_irc_loop_t loop;
    char error[256];
    const char frame[] = "PING :libera.chat\n";

    chain_bot_irc_runtime_init(&runtime);
    runtime.connected = 1;
    chain_bot_irc_loop_init(&loop, &runtime);

    return !chain_bot_irc_loop_feed(
               &loop, frame, sizeof(frame) - 1U, error, sizeof(error)) &&
           loop.pending_length == 0U;
}

static int test_empty_frame_is_rejected(void)
{
    chain_bot_irc_runtime_t runtime;
    chain_bot_irc_loop_t loop;
    char error[256];
    const char frame[] = "\r\n";

    chain_bot_irc_runtime_init(&runtime);
    runtime.connected = 1;
    chain_bot_irc_loop_init(&loop, &runtime);

    return !chain_bot_irc_loop_feed(
               &loop, frame, sizeof(frame) - 1U, error, sizeof(error));
}

static int test_overlong_frame_is_rejected(void)
{
    chain_bot_irc_runtime_t runtime;
    chain_bot_irc_loop_t loop;
    char error[256];
    char frame[CHAIN_BOT_IRC_LINE_MAX + 2U];

    chain_bot_irc_runtime_init(&runtime);
    runtime.connected = 1;
    chain_bot_irc_loop_init(&loop, &runtime);
    memset(frame, 'A', sizeof(frame));

    return !chain_bot_irc_loop_feed(
               &loop, frame, sizeof(frame), error, sizeof(error)) &&
           loop.pending_length == 0U;
}

static int test_zero_length_feed_is_valid(void)
{
    chain_bot_irc_runtime_t runtime;
    chain_bot_irc_loop_t loop;
    char error[256];

    chain_bot_irc_runtime_init(&runtime);
    chain_bot_irc_loop_init(&loop, &runtime);

    return chain_bot_irc_loop_feed(&loop, NULL, 0U, error, sizeof(error)) &&
           loop.pending_length == 0U;
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
    RUN_TEST(test_feed_rejects_invalid_arguments);
    RUN_TEST(test_partial_frame_is_preserved);
    RUN_TEST(test_bare_lf_is_rejected);
    RUN_TEST(test_empty_frame_is_rejected);
    RUN_TEST(test_overlong_frame_is_rejected);
    RUN_TEST(test_zero_length_feed_is_valid);

#undef RUN_TEST

    printf("Chain Bot IRC receive-loop tests: %u/%u passed.\n", passed, executed);
    return passed == executed ? 0 : 1;
}
