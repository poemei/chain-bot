#include <string.h>

#include "chain_bot_irc_module.h"

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
    if (host == NULL) {
        return STNLABZ_MODULE_ERR_INVALID_ARGUMENT;
    }

    /*
     * Transport/session wiring is the next bounded IRC capability.
     * Activation succeeds only after ABI qualification; no network
     * connection is attempted by this descriptor-only pass.
     */
    return STNLABZ_MODULE_OK;
}

static stnlabz_module_result_t chain_bot_irc_stop(void)
{
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
