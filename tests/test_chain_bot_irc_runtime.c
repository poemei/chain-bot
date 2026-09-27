#include <stdio.h>
#include <string.h>

#include "chain_bot_irc_runtime.h"


static int test_init(void)
{
    chain_bot_irc_runtime_t runtime;

    memset(
        &runtime,
        0xff,
        sizeof(runtime)
    );


    chain_bot_irc_runtime_init(
        &runtime
    );


    return
        runtime.connected == 0 &&
        runtime.transport.socket_fd == -1 &&
        runtime.transport.tls_context == NULL &&
        runtime.transport.tls_session == NULL &&
        runtime.session.transport == &runtime.transport &&
        runtime.session.registered == 0 &&
        runtime.session.joined == 0;
}


static int test_connect_rejects_null_runtime(void)
{
    chain_bot_irc_config_t config;
    char error[256];

    memset(
        &config,
        0,
        sizeof(config)
    );


    return !chain_bot_irc_runtime_connect(
        NULL,
        &config,
        error,
        sizeof(error)
    );
}


static int test_connect_rejects_null_config(void)
{
    chain_bot_irc_runtime_t runtime;
    char error[256];

    chain_bot_irc_runtime_init(
        &runtime
    );


    return !chain_bot_irc_runtime_connect(
        &runtime,
        NULL,
        error,
        sizeof(error)
    );
}


static int test_connect_requires_tls(void)
{
    chain_bot_irc_runtime_t runtime;
    chain_bot_irc_config_t config;
    char error[256];

    chain_bot_irc_runtime_init(
        &runtime
    );

    memset(
        &config,
        0,
        sizeof(config)
    );


    (void)snprintf(
        config.host,
        sizeof(config.host),
        "%s",
        "irc.libera.chat"
    );

    config.port = 6697U;

    (void)snprintf(
        config.nick,
        sizeof(config.nick),
        "%s",
        "chain-bot"
    );

    (void)snprintf(
        config.channel,
        sizeof(config.channel),
        "%s",
        "#STNC-Chain"
    );

    (void)snprintf(
        config.password,
        sizeof(config.password),
        "%s",
        "test-only-password"
    );

    config.tls = 0;


    return !chain_bot_irc_runtime_connect(
        &runtime,
        &config,
        error,
        sizeof(error)
    );
}


static int test_connect_requires_password(void)
{
    chain_bot_irc_runtime_t runtime;
    chain_bot_irc_config_t config;
    char error[256];

    chain_bot_irc_runtime_init(
        &runtime
    );

    memset(
        &config,
        0,
        sizeof(config)
    );


    (void)snprintf(
        config.host,
        sizeof(config.host),
        "%s",
        "irc.libera.chat"
    );

    config.port = 6697U;
    config.tls = 1;

    (void)snprintf(
        config.nick,
        sizeof(config.nick),
        "%s",
        "chain-bot"
    );

    (void)snprintf(
        config.channel,
        sizeof(config.channel),
        "%s",
        "#STNC-Chain"
    );


    return !chain_bot_irc_runtime_connect(
        &runtime,
        &config,
        error,
        sizeof(error)
    );
}


static int test_receive_requires_connection(void)
{
    chain_bot_irc_runtime_t runtime;
    char buffer[256];
    char error[256];
    size_t received = 0U;

    chain_bot_irc_runtime_init(
        &runtime
    );


    return !chain_bot_irc_runtime_receive(
        &runtime,
        buffer,
        sizeof(buffer),
        &received,
        error,
        sizeof(error)
    );
}


static int test_handle_requires_connection(void)
{
    chain_bot_irc_runtime_t runtime;
    char error[256];

    chain_bot_irc_runtime_init(
        &runtime
    );


    return !chain_bot_irc_runtime_handle_line(
        &runtime,
        "PING :libera.chat",
        error,
        sizeof(error)
    );
}


static int test_announce_requires_connection(void)
{
    chain_bot_irc_runtime_t runtime;
    char error[256];

    chain_bot_irc_runtime_init(
        &runtime
    );


    return !chain_bot_irc_runtime_announce(
        &runtime,
        "test announcement",
        error,
        sizeof(error)
    );
}


static int test_close_is_idempotent(void)
{
    chain_bot_irc_runtime_t runtime;

    chain_bot_irc_runtime_init(
        &runtime
    );


    chain_bot_irc_runtime_close(
        &runtime
    );

    chain_bot_irc_runtime_close(
        &runtime
    );


    return
        runtime.connected == 0 &&
        runtime.transport.socket_fd == -1 &&
        runtime.transport.tls_context == NULL &&
        runtime.transport.tls_session == NULL &&
        runtime.session.transport == &runtime.transport;
}


int main(void)
{
    unsigned int executed = 0U;
    unsigned int passed = 0U;


#define RUN_TEST(test_fn)                                      \
    do {                                                       \
        ++executed;                                            \
        if ((test_fn)()) {                                     \
            ++passed;                                          \
            printf("[PASS] %s\n", #test_fn);                   \
        } else {                                               \
            printf("[FAIL] %s\n", #test_fn);                   \
        }                                                      \
    } while (0)


    RUN_TEST(test_init);

    RUN_TEST(
        test_connect_rejects_null_runtime
    );

    RUN_TEST(
        test_connect_rejects_null_config
    );

    RUN_TEST(
        test_connect_requires_tls
    );

    RUN_TEST(
        test_connect_requires_password
    );

    RUN_TEST(
        test_receive_requires_connection
    );

    RUN_TEST(
        test_handle_requires_connection
    );

    RUN_TEST(
        test_announce_requires_connection
    );

    RUN_TEST(
        test_close_is_idempotent
    );


#undef RUN_TEST


    printf(
        "Chain Bot IRC runtime tests: %u/%u passed.\n",
        passed,
        executed
    );


    return passed == executed ? 0 : 1;
}