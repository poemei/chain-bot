#include <stdio.h>
#include <string.h>

#include "chain_bot_irc_transport.h"

static int test_init(void)
{
    chain_bot_irc_transport_t transport;

    memset(&transport, 0xff, sizeof(transport));
    chain_bot_irc_transport_init(&transport);

    return transport.socket_fd == -1 &&
           transport.tls_context == NULL &&
           transport.tls_session == NULL;
}

static int test_connect_rejects_invalid_arguments(void)
{
    chain_bot_irc_transport_t transport;
    char error[256];

    chain_bot_irc_transport_init(&transport);

    return !chain_bot_irc_transport_connect(NULL, "irc.libera.chat", 6697U,
                                             error, sizeof(error)) &&
           !chain_bot_irc_transport_connect(&transport, NULL, 6697U,
                                             error, sizeof(error)) &&
           !chain_bot_irc_transport_connect(&transport, "", 6697U,
                                             error, sizeof(error)) &&
           !chain_bot_irc_transport_connect(&transport, "irc.libera.chat", 0U,
                                             error, sizeof(error));
}

static int test_send_rejects_closed_transport(void)
{
    chain_bot_irc_transport_t transport;
    char error[256];
    const char message[] = "PING\r\n";

    chain_bot_irc_transport_init(&transport);

    return !chain_bot_irc_transport_send(&transport,
                                          message,
                                          sizeof(message) - 1U,
                                          error,
                                          sizeof(error));
}

static int test_receive_rejects_closed_transport(void)
{
    chain_bot_irc_transport_t transport;
    char buffer[32];
    char error[256];
    size_t received = 0U;

    chain_bot_irc_transport_init(&transport);

    return !chain_bot_irc_transport_receive(&transport,
                                             buffer,
                                             sizeof(buffer),
                                             &received,
                                             error,
                                             sizeof(error));
}

static int test_close_is_idempotent(void)
{
    chain_bot_irc_transport_t transport;

    chain_bot_irc_transport_init(&transport);
    chain_bot_irc_transport_close(&transport);
    chain_bot_irc_transport_close(&transport);

    return transport.socket_fd == -1 &&
           transport.tls_context == NULL &&
           transport.tls_session == NULL;
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
    RUN_TEST(test_connect_rejects_invalid_arguments);
    RUN_TEST(test_send_rejects_closed_transport);
    RUN_TEST(test_receive_rejects_closed_transport);
    RUN_TEST(test_close_is_idempotent);

#undef RUN_TEST

    printf("Chain Bot IRC transport tests: %u/%u passed.\n", passed, executed);
    return passed == executed ? 0 : 1;
}
