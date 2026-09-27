#define _POSIX_C_SOURCE 200112L

#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include <openssl/err.h>
#include <openssl/ssl.h>

#include "chain_bot_irc_transport.h"

static void set_error(char *error, size_t error_size, const char *message)
{
    if (error != NULL && error_size > 0U) {
        (void)snprintf(error, error_size, "%s", message);
    }
}

static void set_tls_error(char *error, size_t error_size, const char *operation)
{
    unsigned long code = ERR_get_error();
    char detail[160];

    if (code == 0UL) {
        set_error(error, error_size, operation);
        return;
    }

    ERR_error_string_n(code, detail, sizeof(detail));
    if (error != NULL && error_size > 0U) {
        (void)snprintf(error, error_size, "%s: %s", operation, detail);
    }
}

void chain_bot_irc_transport_init(chain_bot_irc_transport_t *transport)
{
    if (transport == NULL) {
        return;
    }

    transport->socket_fd = -1;
    transport->tls_context = NULL;
    transport->tls_session = NULL;
}

int chain_bot_irc_transport_connect(
    chain_bot_irc_transport_t *transport,
    const char *host,
    unsigned short port,
    char *error,
    size_t error_size)
{
    struct addrinfo hints;
    struct addrinfo *addresses = NULL;
    struct addrinfo *address;
    char service[6];
    SSL_CTX *context = NULL;
    SSL *session = NULL;
    int socket_fd = -1;
    int status;

    if (transport == NULL || host == NULL || host[0] == '\0' || port == 0U) {
        set_error(error, error_size, "invalid IRC transport connection request");
        return 0;
    }

    chain_bot_irc_transport_close(transport);

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    (void)snprintf(service, sizeof(service), "%u", (unsigned int)port);
    status = getaddrinfo(host, service, &hints, &addresses);
    if (status != 0) {
        set_error(error, error_size, "unable to resolve IRC server");
        return 0;
    }

    for (address = addresses; address != NULL; address = address->ai_next) {
        socket_fd = socket(address->ai_family, address->ai_socktype, address->ai_protocol);
        if (socket_fd < 0) {
            continue;
        }

        if (connect(socket_fd, address->ai_addr, address->ai_addrlen) == 0) {
            break;
        }

        (void)close(socket_fd);
        socket_fd = -1;
    }

    freeaddrinfo(addresses);

    if (socket_fd < 0) {
        set_error(error, error_size, "unable to connect to IRC server");
        return 0;
    }

    context = SSL_CTX_new(TLS_client_method());
    if (context == NULL) {
        set_tls_error(error, error_size, "unable to create IRC TLS context");
        (void)close(socket_fd);
        return 0;
    }

    SSL_CTX_set_verify(context, SSL_VERIFY_PEER, NULL);
    if (SSL_CTX_set_default_verify_paths(context) != 1) {
        set_tls_error(error, error_size, "unable to load system TLS trust store");
        SSL_CTX_free(context);
        (void)close(socket_fd);
        return 0;
    }

    session = SSL_new(context);
    if (session == NULL) {
        set_tls_error(error, error_size, "unable to create IRC TLS session");
        SSL_CTX_free(context);
        (void)close(socket_fd);
        return 0;
    }

    if (SSL_set_tlsext_host_name(session, host) != 1 ||
        SSL_set1_host(session, host) != 1 ||
        SSL_set_fd(session, socket_fd) != 1) {
        set_tls_error(error, error_size, "unable to configure IRC TLS session");
        SSL_free(session);
        SSL_CTX_free(context);
        (void)close(socket_fd);
        return 0;
    }

    if (SSL_connect(session) != 1) {
        set_tls_error(error, error_size, "IRC TLS handshake failed");
        SSL_free(session);
        SSL_CTX_free(context);
        (void)close(socket_fd);
        return 0;
    }

    if (SSL_get_verify_result(session) != X509_V_OK) {
        set_error(error, error_size, "IRC TLS certificate verification failed");
        (void)SSL_shutdown(session);
        SSL_free(session);
        SSL_CTX_free(context);
        (void)close(socket_fd);
        return 0;
    }

    transport->socket_fd = socket_fd;
    transport->tls_context = context;
    transport->tls_session = session;
    return 1;
}

int chain_bot_irc_transport_send(
    chain_bot_irc_transport_t *transport,
    const void *data,
    size_t length,
    char *error,
    size_t error_size)
{
    SSL *session;
    const unsigned char *cursor = (const unsigned char *)data;
    size_t remaining = length;

    if (transport == NULL || transport->tls_session == NULL ||
        (data == NULL && length > 0U)) {
        set_error(error, error_size, "invalid IRC TLS send request");
        return 0;
    }

    session = (SSL *)transport->tls_session;
    while (remaining > 0U) {
        int chunk = remaining > 16384U ? 16384 : (int)remaining;
        int sent = SSL_write(session, cursor, chunk);

        if (sent <= 0) {
            set_tls_error(error, error_size, "IRC TLS write failed");
            return 0;
        }

        cursor += (size_t)sent;
        remaining -= (size_t)sent;
    }

    return 1;
}

int chain_bot_irc_transport_receive(
    chain_bot_irc_transport_t *transport,
    void *buffer,
    size_t buffer_size,
    size_t *received,
    char *error,
    size_t error_size)
{
    SSL *session;
    int count;

    if (transport == NULL || transport->tls_session == NULL ||
        buffer == NULL || buffer_size == 0U || received == NULL) {
        set_error(error, error_size, "invalid IRC TLS receive request");
        return 0;
    }

    session = (SSL *)transport->tls_session;
    count = SSL_read(session,
                     buffer,
                     buffer_size > 2147483647U ? 2147483647 : (int)buffer_size);
    if (count <= 0) {
        set_tls_error(error, error_size, "IRC TLS read failed");
        return 0;
    }

    *received = (size_t)count;
    return 1;
}

void chain_bot_irc_transport_close(chain_bot_irc_transport_t *transport)
{
    SSL *session;
    SSL_CTX *context;

    if (transport == NULL) {
        return;
    }

    session = (SSL *)transport->tls_session;
    context = (SSL_CTX *)transport->tls_context;

    if (session != NULL) {
        (void)SSL_shutdown(session);
        SSL_free(session);
    }

    if (context != NULL) {
        SSL_CTX_free(context);
    }

    if (transport->socket_fd >= 0) {
        (void)close(transport->socket_fd);
    }

    chain_bot_irc_transport_init(transport);
}
