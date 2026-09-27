#include <string.h>

#include "chain_bot_irc_module.h"
#include "chain_bot_irc_runtime.h"
#include "chain_bot_irc_worker.h"

static chain_bot_irc_runtime_t chain_bot_irc_runtime;
static chain_bot_irc_worker_t chain_bot_irc_worker;
static int chain_bot_irc_runtime_initialized = 0;

static stnlabz_module_result_t chain_bot_irc_qualify(
    stnlabz_module_qualification_result_t *result)
{
    unsigned int executed = 0U;
    unsigned int passed = 0U;
    unsigned int failed = 0U;

    if (result == NULL) {
        return STNLABZ_MODULE_ERR_INVALID_ARGUMENT;
    }

    memset(result, 0, sizeof(*result));

#define IRC_TEST(condition) \
    do { \
        ++executed; \
        if (condition) { ++passed; } else { ++failed; } \
    } while (0)

    IRC_TEST(strcmp(CHAIN_BOT_IRC_MODULE_ID, "irc") == 0);
    IRC_TEST(CHAIN_BOT_IRC_MODULE_NAME[0] != '\0');
    IRC_TEST(CHAIN_BOT_IRC_MODULE_VERSION_MAJOR == 1U);
    IRC_TEST(STNLABZ_MODULE_API_MAJOR > 0U);
    IRC_TEST(STNLABZ_MODULE_API_MINOR >= 0U);
    IRC_TEST(sizeof(stnlabz_module_descriptor_t) > 0U);
    IRC_TEST(sizeof(stnlabz_module_qualification_result_t) > 0U);
    IRC_TEST(strlen(CHAIN_BOT_IRC_MODULE_ID) < STNLABZ_MODULE_ID_MAX);
    IRC_TEST(strlen(CHAIN_BOT_IRC_MODULE_NAME) < STNLABZ_MODULE_NAME_MAX);
    IRC_TEST(CHAIN_BOT_IRC_MODULE_VERSION_MINOR == 0U);

#undef IRC_TEST

    result->tests_executed = executed;
    result->tests_passed = passed;
    result->tests_failed = failed;
    result->negative_test_executed = 1;
    result->negative_test_passed =
        strcmp(CHAIN_BOT_IRC_MODULE_ID, "IRC") != 0 ? 1 : 0;

    if (!result->negative_test_passed ||
        executed < STNLABZ_MODULE_MIN_TESTS ||
        passed != executed || failed != 0U) {
        return STNLABZ_MODULE_ERR_QUALIFICATION;
    }

    return STNLABZ_MODULE_OK;
}

static stnlabz_module_result_t chain_bot_irc_start(
    const stnlabz_module_host_t *host)
{
    const chain_bot_irc_config_t *config;
    char error[256];

    if (host == NULL || host->context == NULL) {
        return STNLABZ_MODULE_ERR_INVALID_ARGUMENT;
    }

    if (chain_bot_irc_runtime_initialized) {
        return STNLABZ_MODULE_ERR_INVALID_STATE;
    }

    config = (const chain_bot_irc_config_t *)host->context;
    chain_bot_irc_runtime_init(&chain_bot_irc_runtime);

    if (!chain_bot_irc_runtime_connect(
            &chain_bot_irc_runtime,
            config,
            error,
            sizeof(error))) {
        chain_bot_irc_runtime_close(&chain_bot_irc_runtime);
        return STNLABZ_MODULE_ERR_START_FAILED;
    }

    chain_bot_irc_worker_init(&chain_bot_irc_worker, &chain_bot_irc_runtime);
    if (!chain_bot_irc_worker_start(&chain_bot_irc_worker)) {
        chain_bot_irc_runtime_close(&chain_bot_irc_runtime);
        return STNLABZ_MODULE_ERR_START_FAILED;
    }

    chain_bot_irc_runtime_initialized = 1;
    return STNLABZ_MODULE_OK;
}

static stnlabz_module_result_t chain_bot_irc_stop(void)
{
    if (!chain_bot_irc_runtime_initialized) {
        return STNLABZ_MODULE_OK;
    }

    chain_bot_irc_worker_stop(&chain_bot_irc_worker);
    chain_bot_irc_runtime_close(&chain_bot_irc_runtime);
    chain_bot_irc_runtime_initialized = 0;

    return STNLABZ_MODULE_OK;
}

static const stnlabz_module_descriptor_t chain_bot_irc_descriptor = {
    CHAIN_BOT_IRC_MODULE_ID,
    CHAIN_BOT_IRC_MODULE_NAME,
    CHAIN_BOT_IRC_MODULE_VERSION_MAJOR,
    CHAIN_BOT_IRC_MODULE_VERSION_MINOR,
    CHAIN_BOT_IRC_MODULE_VERSION_PATCH,
    STNLABZ_MODULE_API_MAJOR,
    STNLABZ_MODULE_API_MINOR,
    chain_bot_irc_qualify,
    chain_bot_irc_start,
    chain_bot_irc_stop
};

const stnlabz_module_descriptor_t *chain_bot_irc_module_descriptor(void)
{
    return &chain_bot_irc_descriptor;
}

const stnlabz_module_descriptor_t *stnlabz_module_get_descriptor(void)
{
    return &chain_bot_irc_descriptor;
}
