#ifndef CHAIN_BOT_IRC_TRANSPORT_H
#define CHAIN_BOT_IRC_TRANSPORT_H

#include <stddef.h>

typedef struct chain_bot_irc_transport {
    int socket_fd;
    void *tls_context;
    void *tls_session;
} chain_bot_irc_transport_t;

void chain_bot_irc_transport_init(chain_bot_irc_transport_t *transport);

int chain_bot_irc_transport_connect(
    chain_bot_irc_transport_t *transport,
    const char *host,
    unsigned short port,
    char *error,
    size_t error_size);

int chain_bot_irc_transport_send(
    chain_bot_irc_transport_t *transport,
    const void *data,
    size_t length,
    char *error,
    size_t error_size);

int chain_bot_irc_transport_receive(
    chain_bot_irc_transport_t *transport,
    void *buffer,
    size_t buffer_size,
    size_t *received,
    char *error,
    size_t error_size);

void chain_bot_irc_transport_close(chain_bot_irc_transport_t *transport);

#endif
