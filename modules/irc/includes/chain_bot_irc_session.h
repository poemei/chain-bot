#ifndef CHAIN_BOT_IRC_SESSION_H
#define CHAIN_BOT_IRC_SESSION_H

#include <stddef.h>

#include "chain_bot_irc_transport.h"

#define CHAIN_BOT_IRC_LINE_MAX 512

typedef struct chain_bot_irc_session {
    chain_bot_irc_transport_t *transport;
    char nick[64];
    char channel[128];
    int registered;
    int joined;
} chain_bot_irc_session_t;

void chain_bot_irc_session_init(
    chain_bot_irc_session_t *session,
    chain_bot_irc_transport_t *transport);

int chain_bot_irc_session_begin(
    chain_bot_irc_session_t *session,
    const char *password,
    const char *nick,
    const char *channel,
    char *error,
    size_t error_size);

int chain_bot_irc_session_handle_line(
    chain_bot_irc_session_t *session,
    const char *line,
    char *error,
    size_t error_size);

int chain_bot_irc_session_privmsg(
    chain_bot_irc_session_t *session,
    const char *message,
    char *error,
    size_t error_size);

int chain_bot_irc_session_privmsg_to(
    chain_bot_irc_session_t *session,
    const char *target,
    const char *message,
    char *error,
    size_t error_size);

int chain_bot_irc_session_quit(
    chain_bot_irc_session_t *session,
    const char *reason,
    char *error,
    size_t error_size);

#endif
