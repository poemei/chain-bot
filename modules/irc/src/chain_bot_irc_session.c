#include <stdio.h>
#include <string.h>

#include "chain_bot_irc_session.h"

static void set_error(char *error, size_t error_size, const char *message)
{
    if (error != NULL && error_size > 0U) {
        (void)snprintf(error, error_size, "%s", message);
    }
}

static int valid_token(const char *value, size_t max_length)
{
    size_t length;

    if (value == NULL || value[0] == '\0') {
        return 0;
    }

    length = strlen(value);
    return length < max_length && strchr(value, '\r') == NULL &&
           strchr(value, '\n') == NULL && strchr(value, ' ') == NULL;
}

static int send_line(chain_bot_irc_session_t *session,
                     const char *line,
                     char *error,
                     size_t error_size)
{
    if (session == NULL || session->transport == NULL || line == NULL) {
        set_error(error, error_size, "invalid IRC line send request");
        return 0;
    }

    return chain_bot_irc_transport_send(session->transport,
                                        line,
                                        strlen(line),
                                        error,
                                        error_size);
}

void chain_bot_irc_session_init(chain_bot_irc_session_t *session,
                                chain_bot_irc_transport_t *transport)
{
    if (session == NULL) {
        return;
    }

    memset(session, 0, sizeof(*session));
    session->transport = transport;
}

int chain_bot_irc_session_begin(chain_bot_irc_session_t *session,
                                const char *password,
                                const char *nick,
                                const char *channel,
                                char *error,
                                size_t error_size)
{
    char line[CHAIN_BOT_IRC_LINE_MAX + 1U];
    int count;

    if (session == NULL || session->transport == NULL ||
        !valid_token(password, 256U) || !valid_token(nick, sizeof(session->nick)) ||
        !valid_token(channel, sizeof(session->channel)) || channel[0] != '#') {
        set_error(error, error_size, "invalid IRC registration request");
        return 0;
    }

    (void)snprintf(session->nick, sizeof(session->nick), "%s", nick);
    (void)snprintf(session->channel, sizeof(session->channel), "%s", channel);
    session->registered = 0;
    session->joined = 0;

    count = snprintf(line, sizeof(line), "PASS %s\r\n", password);
    if (count < 0 || (size_t)count >= sizeof(line) ||
        !send_line(session, line, error, error_size)) {
        return 0;
    }

    count = snprintf(line, sizeof(line), "NICK %s\r\n", nick);
    if (count < 0 || (size_t)count >= sizeof(line) ||
        !send_line(session, line, error, error_size)) {
        return 0;
    }

    count = snprintf(line, sizeof(line),
                     "USER %s 0 * :STN Chain Bot\r\n", nick);
    if (count < 0 || (size_t)count >= sizeof(line) ||
        !send_line(session, line, error, error_size)) {
        return 0;
    }

    return 1;
}

int chain_bot_irc_session_handle_line(chain_bot_irc_session_t *session,
                                      const char *line,
                                      char *error,
                                      size_t error_size)
{
    char output[CHAIN_BOT_IRC_LINE_MAX + 1U];
    const char *payload;
    int count;

    if (session == NULL || session->transport == NULL || line == NULL ||
        strchr(line, '\r') != NULL || strchr(line, '\n') != NULL) {
        set_error(error, error_size, "invalid IRC input line");
        return 0;
    }

    if (strncmp(line, "PING ", 5U) == 0) {
        payload = line + 5U;
        if (payload[0] == '\0') {
            set_error(error, error_size, "invalid IRC PING");
            return 0;
        }

        count = snprintf(output, sizeof(output), "PONG %s\r\n", payload);
        return count >= 0 && (size_t)count < sizeof(output) &&
               send_line(session, output, error, error_size);
    }

    if (strstr(line, " 001 ") != NULL && !session->registered) {
        session->registered = 1;
        count = snprintf(output, sizeof(output), "JOIN %s\r\n", session->channel);
        if (count < 0 || (size_t)count >= sizeof(output) ||
            !send_line(session, output, error, error_size)) {
            session->registered = 0;
            return 0;
        }
        return 1;
    }

    if (strstr(line, " JOIN ") != NULL && strstr(line, session->nick) != NULL &&
        strstr(line, session->channel) != NULL) {
        session->joined = 1;
    }

    return 1;
}

int chain_bot_irc_session_privmsg(chain_bot_irc_session_t *session,
                                  const char *message,
                                  char *error,
                                  size_t error_size)
{
    char line[CHAIN_BOT_IRC_LINE_MAX + 1U];
    int count;

    if (session == NULL || session->transport == NULL || !session->joined ||
        message == NULL || message[0] == '\0' || strchr(message, '\r') != NULL ||
        strchr(message, '\n') != NULL) {
        set_error(error, error_size, "invalid IRC channel message request");
        return 0;
    }

    count = snprintf(line, sizeof(line), "PRIVMSG %s :%s\r\n",
                     session->channel, message);
    if (count < 0 || (size_t)count >= sizeof(line)) {
        set_error(error, error_size, "IRC channel message exceeds protocol limit");
        return 0;
    }

    return send_line(session, line, error, error_size);
}

int chain_bot_irc_session_quit(chain_bot_irc_session_t *session,
                               const char *reason,
                               char *error,
                               size_t error_size)
{
    char line[CHAIN_BOT_IRC_LINE_MAX + 1U];
    int count;

    if (session == NULL || session->transport == NULL || reason == NULL ||
        strchr(reason, '\r') != NULL || strchr(reason, '\n') != NULL) {
        set_error(error, error_size, "invalid IRC quit request");
        return 0;
    }

    count = snprintf(line, sizeof(line), "QUIT :%s\r\n", reason);
    if (count < 0 || (size_t)count >= sizeof(line)) {
        set_error(error, error_size, "IRC quit reason exceeds protocol limit");
        return 0;
    }

    if (!send_line(session, line, error, error_size)) {
        return 0;
    }

    session->registered = 0;
    session->joined = 0;
    return 1;
}
