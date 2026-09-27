#include <stdio.h>
#include <string.h>

#include "chain_bot_config.h"

static int write_fixture(const char *path, const char *content)
{
    FILE *file = fopen(path, "wb");
    size_t length;

    if (file == NULL) {
        return 0;
    }

    length = strlen(content);
    if (fwrite(content, 1U, length, file) != length) {
        fclose(file);
        return 0;
    }

    return fclose(file) == 0;
}

static int test_valid_config(void)
{
    const char *path = "test_chain_bot_valid.json";
    const char *json =
        "{\n"
        "  \"chain\": {\"host\": \"127.0.0.1\", \"port\": 18473},\n"
        "  \"irc\": {\"host\": \"irc.libera.chat\", \"port\": 6697, \"tls\": true, "
        "\"channel\": \"#STNC-Chain\", \"nick\": \"chain-bot\", \"password\": \"test-only-password\"}\n"
        "}\n";
    chain_bot_config_t config;
    char error[256];
    int ok;

    if (!write_fixture(path, json)) {
        return 0;
    }

    ok = chain_bot_config_load(path, &config, error, sizeof(error));
    (void)remove(path);

    return ok && strcmp(config.chain.host, "127.0.0.1") == 0 &&
           config.chain.port == 18473U &&
           strcmp(config.irc.host, "irc.libera.chat") == 0 &&
           config.irc.port == 6697U && config.irc.tls == 1 &&
           strcmp(config.irc.channel, "#STNC-Chain") == 0 &&
           strcmp(config.irc.nick, "chain-bot") == 0 &&
           strcmp(config.irc.password, "test-only-password") == 0;
}

static int test_tls_required(void)
{
    const char *path = "test_chain_bot_tls.json";
    const char *json =
        "{\"chain\":{\"host\":\"127.0.0.1\",\"port\":18473},"
        "\"irc\":{\"host\":\"irc.libera.chat\",\"port\":6697,\"tls\":false,"
        "\"channel\":\"#STNC-Chain\",\"nick\":\"chain-bot\",\"password\":\"test-only-password\"}}";
    chain_bot_config_t config;
    char error[256];
    int ok;

    if (!write_fixture(path, json)) {
        return 0;
    }

    ok = chain_bot_config_load(path, &config, error, sizeof(error));
    (void)remove(path);
    return !ok;
}

static int test_password_required(void)
{
    const char *path = "test_chain_bot_password.json";
    const char *json =
        "{\"chain\":{\"host\":\"127.0.0.1\",\"port\":18473},"
        "\"irc\":{\"host\":\"irc.libera.chat\",\"port\":6697,\"tls\":true,"
        "\"channel\":\"#STNC-Chain\",\"nick\":\"chain-bot\"}}";
    chain_bot_config_t config;
    char error[256];
    int ok;

    if (!write_fixture(path, json)) {
        return 0;
    }

    ok = chain_bot_config_load(path, &config, error, sizeof(error));
    (void)remove(path);
    return !ok;
}

static int test_missing_file(void)
{
    chain_bot_config_t config;
    char error[256];

    return !chain_bot_config_load("does-not-exist.json", &config, error, sizeof(error));
}

static int test_invalid_arguments(void)
{
    chain_bot_config_t config;
    char error[256];

    return !chain_bot_config_load(NULL, &config, error, sizeof(error)) &&
           !chain_bot_config_load("unused.json", NULL, error, sizeof(error));
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

    RUN_TEST(test_valid_config);
    RUN_TEST(test_tls_required);
    RUN_TEST(test_password_required);
    RUN_TEST(test_missing_file);
    RUN_TEST(test_invalid_arguments);

#undef RUN_TEST

    printf("Chain Bot configuration tests: %u/%u passed.\n", passed, executed);
    return passed == executed ? 0 : 1;
}
