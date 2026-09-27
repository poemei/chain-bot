#include <stdio.h>
#include <string.h>

#include "chain_bot_irc_module.h"

static int test_descriptor(void)
{
    const stnlabz_module_descriptor_t *descriptor =
        chain_bot_irc_module_descriptor();

    return descriptor != NULL &&
           strcmp(descriptor->id, "irc") == 0 &&
           strcmp(descriptor->name, "Chain Bot IRC") == 0 &&
           descriptor->version_major == 1U &&
           descriptor->required_core_api_major == STNLABZ_MODULE_API_MAJOR &&
           descriptor->required_core_api_minor == STNLABZ_MODULE_API_MINOR &&
           descriptor->qualify != NULL &&
           descriptor->start != NULL &&
           descriptor->stop != NULL;
}

static int test_export(void)
{
    return stnlabz_module_get_descriptor() ==
           chain_bot_irc_module_descriptor();
}

static int test_qualification(void)
{
    const stnlabz_module_descriptor_t *descriptor =
        chain_bot_irc_module_descriptor();
    stnlabz_module_qualification_result_t result;

    if (descriptor->qualify(&result) != STNLABZ_MODULE_OK) {
        return 0;
    }

    return result.tests_executed >= STNLABZ_MODULE_MIN_TESTS &&
           result.tests_passed == result.tests_executed &&
           result.tests_failed == 0U &&
           result.negative_test_executed == 1 &&
           result.negative_test_passed == 1;
}

static int test_qualification_rejects_null(void)
{
    const stnlabz_module_descriptor_t *descriptor =
        chain_bot_irc_module_descriptor();

    return descriptor->qualify(NULL) == STNLABZ_MODULE_ERR_INVALID_ARGUMENT;
}

static int test_start_requires_host(void)
{
    const stnlabz_module_descriptor_t *descriptor =
        chain_bot_irc_module_descriptor();

    return descriptor->start(NULL) == STNLABZ_MODULE_ERR_INVALID_ARGUMENT;
}

static int test_start_and_stop(void)
{
    const stnlabz_module_descriptor_t *descriptor =
        chain_bot_irc_module_descriptor();
    stnlabz_module_host_t host;

    memset(&host, 0, sizeof(host));

    return descriptor->start(&host) == STNLABZ_MODULE_OK &&
           descriptor->stop() == STNLABZ_MODULE_OK;
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

    RUN_TEST(test_descriptor);
    RUN_TEST(test_export);
    RUN_TEST(test_qualification);
    RUN_TEST(test_qualification_rejects_null);
    RUN_TEST(test_start_requires_host);
    RUN_TEST(test_start_and_stop);

#undef RUN_TEST

    printf("Chain Bot IRC module tests: %u/%u passed.\n", passed, executed);
    return passed == executed ? 0 : 1;
}
