#include <stddef.h>
#include <string.h>

#include "chain_bot_irc_runtime.h"


void chain_bot_irc_runtime_init(
    chain_bot_irc_runtime_t *runtime)
{
    if (runtime == NULL) {
        return;
    }

    memset(runtime, 0, sizeof(*runtime));

    chain_bot_irc_transport_init(
        &runtime->transport
    );

    chain_bot_irc_session_init(
        &runtime->session,
        &runtime->transport
    );

    runtime->connected = 0;
}


int chain_bot_irc_runtime_connect(
    chain_bot_irc_runtime_t *runtime,
    const chain_bot_irc_config_t *config,
    char *error,
    size_t error_size)
{
    if (runtime == NULL ||
        config == NULL ||
        config->host[0] == '\0' ||
        config->port == 0U ||
        config->nick[0] == '\0' ||
        config->channel[0] == '\0' ||
        config->password[0] == '\0' ||
        !config->tls) {

        return 0;
    }


    /*
     * Chain Bot IRC is TLS-only.
     *
     * Transport establishes and verifies the TLS
     * connection before IRC registration begins.
     */

    if (!chain_bot_irc_transport_connect(
            &runtime->transport,
            config->host,
            config->port,
            error,
            error_size)) {

        chain_bot_irc_runtime_close(runtime);

        return 0;
    }


    /*
     * Registration credentials are supplied directly
     * from runtime configuration.
     *
     * The password is never logged, copied into a
     * diagnostic buffer, or persisted by this layer.
     */

    if (!chain_bot_irc_session_begin(
            &runtime->session,
            config->password,
            config->nick,
            config->channel,
            error,
            error_size)) {

        chain_bot_irc_runtime_close(runtime);

        return 0;
    }


    runtime->connected = 1;

    return 1;
}


int chain_bot_irc_runtime_receive(
    chain_bot_irc_runtime_t *runtime,
    char *buffer,
    size_t buffer_size,
    size_t *received,
    char *error,
    size_t error_size)
{
    if (runtime == NULL ||
        !runtime->connected ||
        buffer == NULL ||
        buffer_size == 0U ||
        received == NULL) {

        return 0;
    }


    return chain_bot_irc_transport_receive(
        &runtime->transport,
        buffer,
        buffer_size,
        received,
        error,
        error_size
    );
}


int chain_bot_irc_runtime_handle_line(
    chain_bot_irc_runtime_t *runtime,
    const char *line,
    char *error,
    size_t error_size)
{
    if (runtime == NULL ||
        !runtime->connected ||
        line == NULL) {

        return 0;
    }


    return chain_bot_irc_session_handle_line(
        &runtime->session,
        line,
        error,
        error_size
    );
}


int chain_bot_irc_runtime_announce(
    chain_bot_irc_runtime_t *runtime,
    const char *message,
    char *error,
    size_t error_size)
{
    if (runtime == NULL ||
        !runtime->connected ||
        message == NULL ||
        message[0] == '\0') {

        return 0;
    }


    return chain_bot_irc_session_privmsg(
        &runtime->session,
        message,
        error,
        error_size
    );
}


void chain_bot_irc_runtime_close(
    chain_bot_irc_runtime_t *runtime)
{
    char error[256];

    if (runtime == NULL) {
        return;
    }


    /*
     * QUIT is best-effort.
     *
     * Failure to send QUIT must never prevent the
     * transport from being closed.
     */

    if (runtime->connected) {

        (void)chain_bot_irc_session_quit(
            &runtime->session,
            "Chain Bot shutting down",
            error,
            sizeof(error)
        );
    }


    chain_bot_irc_transport_close(
        &runtime->transport
    );


    chain_bot_irc_session_init(
        &runtime->session,
        &runtime->transport
    );


    runtime->connected = 0;
}