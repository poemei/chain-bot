#include <stdio.h>
#include <string.h>

#include "chain_bot_irc_loop.h"

static void set_error(char *error, size_t error_size, const char *message)
{
    if (error != NULL && error_size > 0U) {
        (void)snprintf(error, error_size, "%s", message);
    }
}

void chain_bot_irc_loop_init(
    chain_bot_irc_loop_t *loop,
    chain_bot_irc_runtime_t *runtime)
{
    if (loop == NULL) {
        return;
    }

    memset(loop, 0, sizeof(*loop));
    loop->runtime = runtime;
}

int chain_bot_irc_loop_feed(
    chain_bot_irc_loop_t *loop,
    const char *data,
    size_t length,
    char *error,
    size_t error_size)
{
    size_t index;

    if (loop == NULL || loop->runtime == NULL ||
        (data == NULL && length > 0U)) {
        set_error(error, error_size, "invalid IRC receive-loop input");
        return 0;
    }

    for (index = 0U; index < length; ++index) {
        char byte = data[index];

        if (loop->pending_length >= sizeof(loop->pending)) {
            loop->pending_length = 0U;
            set_error(error, error_size, "IRC receive buffer exceeded");
            return 0;
        }

        loop->pending[loop->pending_length++] = byte;

        if (loop->pending_length >= 2U &&
            loop->pending[loop->pending_length - 2U] == '\r' &&
            loop->pending[loop->pending_length - 1U] == '\n') {
            size_t line_length = loop->pending_length - 2U;
            char line[CHAIN_BOT_IRC_LINE_MAX + 1U];

            if (line_length == 0U || line_length > CHAIN_BOT_IRC_LINE_MAX) {
                loop->pending_length = 0U;
                set_error(error, error_size, "invalid IRC frame length");
                return 0;
            }

            memcpy(line, loop->pending, line_length);
            line[line_length] = '\0';
            loop->pending_length = 0U;

            if (!chain_bot_irc_runtime_handle_line(
                    loop->runtime,
                    line,
                    error,
                    error_size)) {
                return 0;
            }
        } else if (byte == '\n') {
            loop->pending_length = 0U;
            set_error(error, error_size, "IRC frame missing CRLF terminator");
            return 0;
        }
    }

    if (loop->pending_length > CHAIN_BOT_IRC_LINE_MAX) {
        loop->pending_length = 0U;
        set_error(error, error_size, "IRC frame exceeds protocol limit");
        return 0;
    }

    return 1;
}

int chain_bot_irc_loop_receive_once(
    chain_bot_irc_loop_t *loop,
    char *error,
    size_t error_size)
{
    char buffer[1024];
    size_t received = 0U;

    if (loop == NULL || loop->runtime == NULL) {
        set_error(error, error_size, "invalid IRC receive-loop request");
        return 0;
    }

    if (!chain_bot_irc_runtime_receive(
            loop->runtime,
            buffer,
            sizeof(buffer),
            &received,
            error,
            error_size)) {
        return 0;
    }

    if (received == 0U) {
        set_error(error, error_size, "IRC connection closed");
        return 0;
    }

    return chain_bot_irc_loop_feed(
        loop,
        buffer,
        received,
        error,
        error_size);
}
