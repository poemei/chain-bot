#include <stdio.h>
#include <string.h>

#include "chain_bot_slack_module.h"

int main(void)
{
    const stnlabz_module_descriptor_t *descriptor = chain_bot_slack_module_descriptor();
    stnlabz_module_qualification_result_t qualification;
    unsigned int executed = 0U;
    unsigned int passed = 0U;

#define TEST(c) \
    do { \
        ++executed; \
        if (c) { \
            ++passed; \
            printf("[PASS] %s\n", #c); \
        } else { \
            printf("[FAIL] %s\n", #c); \
        } \
    } while (0)

    TEST(descriptor != NULL);
    TEST(descriptor != NULL && strcmp(descriptor->id, CHAIN_BOT_SLACK_MODULE_ID) == 0);
    TEST(descriptor != NULL && strcmp(descriptor->name, CHAIN_BOT_SLACK_MODULE_NAME) == 0);
    TEST(descriptor != NULL && descriptor->version_major == CHAIN_BOT_SLACK_MODULE_VERSION_MAJOR);
    TEST(descriptor != NULL && descriptor->version_minor == CHAIN_BOT_SLACK_MODULE_VERSION_MINOR);
    TEST(descriptor != NULL && descriptor->version_patch == CHAIN_BOT_SLACK_MODULE_VERSION_PATCH);
    TEST(descriptor != NULL && descriptor->required_core_api_major == STNLABZ_MODULE_API_MAJOR);
    TEST(descriptor != NULL && descriptor->required_core_api_minor == STNLABZ_MODULE_API_MINOR);
    TEST(descriptor != NULL && descriptor->qualify != NULL);
    TEST(descriptor != NULL && descriptor->start != NULL);
    TEST(descriptor != NULL && descriptor->stop != NULL);
    TEST(descriptor != NULL && descriptor->qualify(&qualification) == STNLABZ_MODULE_OK);
    TEST(qualification.tests_executed >= STNLABZ_MODULE_MIN_TESTS);
    TEST(qualification.tests_failed == 0U);
    TEST(qualification.negative_test_executed == 1);
    TEST(qualification.negative_test_passed == 1);
    TEST(stnlabz_module_get_descriptor() == descriptor);

#undef TEST

    printf("Chain Bot Slack module tests: %u/%u passed.\n", passed, executed);
    return passed == executed ? 0 : 1;
}
