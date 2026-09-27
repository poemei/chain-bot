#include <stdio.h>
#include <string.h>

#include "chain_bot_irc_session.h"

static int test_init(void)
{
    chain_bot_irc_transport_t transport;
    chain_bot_irc_session_t session;

    chain_bot_irc_transport_init(&transport);
    memset(&session, 0xff, sizeof(session));
    chain_bot_irc_session_init(&session, &transport);

    return session.transport == &transport && session.nick[0] == '\0' &&
           session.channel[0] == '\0' && !session.registered && !session.joined;
}

static int test_begin_rejects_invalid_arguments(void)
{
    chain_bot_irc_transport_t transport;
    chain_bot_irc_session_t session;
    char error[256];

    chain_bot_irc_transport_init(&transport);
    chain_bot_irc_session_init(&session, &transport);

    return !chain_bot_irc_session_begin(NULL, "password", "chain-bot",
                                         "#STNC-Chain", error, sizeof(error)) &&
           !chain_bot_irc_session_begin(&session, "", "chain-bot",
                                         "#STNC-Chain", error, sizeof(error)) &&
           !chain_bot_irc_session_begin(&session, "password", "bad nick",
                                         "#STNC-Chain", error, sizeof(error)) &&
           !chain_bot_irc_session_begin(&session, "password", "chain-bot",
                                         "STNC-Chain", error, sizeof(error)) &&
           !chain_bot_irc_session_begin(&session, "bad\rpassword", "chain-bot",
                                         "#STNC-Chain", error, sizeof(error));
}

static int test_handle_rejects_injected_line(void)
{
    chain_bot_irc_transport_t transport;
    chain_bot_irc_session_t session;
    char error[256];

    chain_bot_irc_transport_init(&transport);
    chain_bot_irc_session_init(&session, &transport);

    return !chain_bot_irc_session_handle_line(
        &session, "PING :x\r\nPRIVMSG #x :bad", error, sizeof(error));
}

static int test_privmsg_requires_join(void)
{
    chain_bot_irc_transport_t transport;
    chain_bot_irc_session_t session;
    char error[256];

    chain_bot_irc_transport_init(&transport);
    chain_bot_irc_session_init(&session, &transport);
    (void)snprintf(session.channel, sizeof(session.channel), "%s", "#STNC-Chain");

    return !chain_bot_irc_session_privmsg(&session, "hello", error, sizeof(error));
}

static int test_privmsg_rejects_line_injection(void)
{
    chain_bot_irc_transport_t transport;
    chain_bot_irc_session_t session;
    char error[256];

    chain_bot_irc_transport_init(&transport);
    chain_bot_irc_session_init(&session, &transport);
    session.joined = 1;
    (void)snprintf(session.channel, sizeof(session.channel), "%s", "#STNC-Chain");

    return !chain_bot_irc_session_privmsg(
        &session, "hello\r\nQUIT :oops", error, sizeof(error));
}

static int test_quit_rejects_line_injection(void)
{
    chain_bot_irc_transport_t transport;
    chain_bot_irc_session_t session;
    char error[256];

    chain_bot_irc_transport_init(&transport);
    chain_bot_irc_session_init(&session, &transport);

    return !chain_bot_irc_session_quit(
        &session, "bye\nJOIN #other", error, sizeof(error));
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
    RUN_TEST(test_begin_rejects_invalid_arguments);
    RUN_TEST(test_handle_rejects_injected_line);
    RUN_TEST(test_privmsg_requires_join);
    RUN_TEST(test_privmsg_rejects_line_injection);
    RUN_TEST(test_quit_rejects_line_injection);

#undef RUN_TEST

    printf("Chain Bot IRC session tests: %u/%u passed.\n", passed, executed);
    return passed == executed ? 0 : 1;
}
