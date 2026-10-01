#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <string.h>
#include <time.h>

#include "chain_bot_log.h"

static FILE *chain_bot_log_file = NULL;

static void set_error(char *error, size_t error_size, const char *message)
{
    if (error != NULL && error_size > 0U) {
        (void)snprintf(error, error_size, "%s", message);
    }
}

static void write_log(const char *level, const char *message, FILE *stream)
{
    time_t now;
    struct tm timestamp;
    char time_text[32];

    if (level == NULL || message == NULL) return;

    now = time(NULL);
    if (localtime_r(&now, &timestamp) != NULL &&
        strftime(time_text, sizeof(time_text), "%Y-%m-%dT%H:%M:%S%z", &timestamp) > 0U) {
        if (chain_bot_log_file != NULL) {
            (void)fprintf(chain_bot_log_file, "%s [%s] %s\n", time_text, level, message);
            (void)fflush(chain_bot_log_file);
        }
        (void)fprintf(stream, "%s [%s] %s\n", time_text, level, message);
    } else {
        if (chain_bot_log_file != NULL) {
            (void)fprintf(chain_bot_log_file, "[%s] %s\n", level, message);
            (void)fflush(chain_bot_log_file);
        }
        (void)fprintf(stream, "[%s] %s\n", level, message);
    }

    (void)fflush(stream);
}

int chain_bot_log_open(const char *path, char *error, size_t error_size)
{
    if (path == NULL || path[0] == '\0') {
        set_error(error, error_size, "invalid log path");
        return 0;
    }

    if (chain_bot_log_file != NULL) {
        (void)fclose(chain_bot_log_file);
        chain_bot_log_file = NULL;
    }

    chain_bot_log_file = fopen(path, "a");
    if (chain_bot_log_file == NULL) {
        set_error(error, error_size, "unable to open Chain Bot log file");
        return 0;
    }

    return 1;
}

void chain_bot_log_info(const char *message)
{
    write_log("INFO", message, stdout);
}

void chain_bot_log_warn(const char *message)
{
    write_log("WARN", message, stderr);
}

void chain_bot_log_error(const char *message)
{
    write_log("ERROR", message, stderr);
}

void chain_bot_log_close(void)
{
    if (chain_bot_log_file != NULL) {
        (void)fflush(chain_bot_log_file);
        (void)fclose(chain_bot_log_file);
        chain_bot_log_file = NULL;
    }
}
