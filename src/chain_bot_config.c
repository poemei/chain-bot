#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "chain_bot_config.h"

#define CHAIN_BOT_CONFIG_FILE_MAX 16384

static void set_error(char *error, size_t error_size, const char *message)
{
    if (error != NULL && error_size > 0U) {
        (void)snprintf(error, error_size, "%s", message);
    }
}

static const char *skip_ws(const char *p)
{
    while (*p != '\0' && isspace((unsigned char)*p)) {
        ++p;
    }
    return p;
}

static const char *find_section(const char *json, const char *section)
{
    char needle[96];
    const char *p;

    if (snprintf(needle, sizeof(needle), "\"%s\"", section) < 0) {
        return NULL;
    }

    p = strstr(json, needle);
    if (p == NULL) {
        return NULL;
    }

    p = strchr(p + strlen(needle), ':');
    if (p == NULL) {
        return NULL;
    }

    p = skip_ws(p + 1);
    return *p == '{' ? p + 1 : NULL;
}

static const char *find_value(const char *section, const char *key)
{
    char needle[96];
    const char *p;
    const char *end;

    if (section == NULL || snprintf(needle, sizeof(needle), "\"%s\"", key) < 0) {
        return NULL;
    }

    end = strchr(section, '}');
    if (end == NULL) {
        return NULL;
    }

    p = strstr(section, needle);
    if (p == NULL || p >= end) {
        return NULL;
    }

    p += strlen(needle);
    p = skip_ws(p);
    if (*p != ':') {
        return NULL;
    }

    p = skip_ws(p + 1);
    return p < end ? p : NULL;
}

static int parse_string(const char *section, const char *key, char *out, size_t out_size)
{
    const char *p = find_value(section, key);
    size_t used = 0U;

    if (p == NULL || *p != '"' || out == NULL || out_size == 0U) {
        return 0;
    }

    ++p;
    while (*p != '\0' && *p != '"') {
        if (*p == '\\') {
            ++p;
            if (*p != '"' && *p != '\\' && *p != '/') {
                return 0;
            }
        }
        if (used + 1U >= out_size) {
            return 0;
        }
        out[used++] = *p++;
    }

    if (*p != '"') {
        return 0;
    }

    out[used] = '\0';
    return 1;
}

static int parse_port(const char *section, const char *key, unsigned short *port)
{
    const char *p = find_value(section, key);
    char *end;
    unsigned long value;

    if (p == NULL || port == NULL) {
        return 0;
    }

    errno = 0;
    value = strtoul(p, &end, 10);
    if (errno != 0 || end == p || value == 0UL || value > 65535UL) {
        return 0;
    }

    *port = (unsigned short)value;
    return 1;
}

static int parse_bool(const char *section, const char *key, int *value)
{
    const char *p = find_value(section, key);

    if (p == NULL || value == NULL) {
        return 0;
    }
    if (strncmp(p, "true", 4U) == 0) {
        *value = 1;
        return 1;
    }
    if (strncmp(p, "false", 5U) == 0) {
        *value = 0;
        return 1;
    }
    return 0;
}

int chain_bot_config_load(const char *path,
                          chain_bot_config_t *config,
                          char *error,
                          size_t error_size)
{
    FILE *file;
    long length;
    size_t read_count;
    char *json;
    const char *chain;
    const char *irc;

    if (path == NULL || config == NULL) {
        set_error(error, error_size, "invalid configuration request");
        return 0;
    }

    file = fopen(path, "rb");
    if (file == NULL) {
        set_error(error, error_size, "unable to open chain_bot.json");
        return 0;
    }

    if (fseek(file, 0L, SEEK_END) != 0 || (length = ftell(file)) < 0L ||
        length > CHAIN_BOT_CONFIG_FILE_MAX || fseek(file, 0L, SEEK_SET) != 0) {
        fclose(file);
        set_error(error, error_size, "unable to read chain_bot.json");
        return 0;
    }

    json = (char *)malloc((size_t)length + 1U);
    if (json == NULL) {
        fclose(file);
        set_error(error, error_size, "unable to allocate configuration buffer");
        return 0;
    }

    read_count = fread(json, 1U, (size_t)length, file);
    fclose(file);
    if (read_count != (size_t)length) {
        free(json);
        set_error(error, error_size, "unable to read chain_bot.json");
        return 0;
    }
    json[read_count] = '\0';

    memset(config, 0, sizeof(*config));
    chain = find_section(json, "chain");
    irc = find_section(json, "irc");

    if (chain == NULL || irc == NULL ||
        !parse_string(chain, "host", config->chain.host, sizeof(config->chain.host)) ||
        !parse_port(chain, "port", &config->chain.port) ||
        !parse_string(irc, "host", config->irc.host, sizeof(config->irc.host)) ||
        !parse_port(irc, "port", &config->irc.port) ||
        !parse_bool(irc, "tls", &config->irc.tls) ||
        !parse_string(irc, "channel", config->irc.channel, sizeof(config->irc.channel)) ||
        !parse_string(irc, "nick", config->irc.nick, sizeof(config->irc.nick)) ||
        !parse_string(irc, "password", config->irc.password, sizeof(config->irc.password))) {
        free(json);
        set_error(error, error_size, "chain_bot.json is missing or contains an invalid setting");
        return 0;
    }

    free(json);

    if (config->chain.host[0] == '\0' || config->irc.host[0] == '\0' ||
        config->irc.channel[0] != '#' || config->irc.nick[0] == '\0' ||
        config->irc.password[0] == '\0' || !config->irc.tls) {
        set_error(error, error_size, "chain_bot.json contains an invalid configuration");
        return 0;
    }

    return 1;
}
