#include <string.h>

#include "chain_bot_slack_module.h"

static int chain_bot_slack_active = 0;

static stnlabz_module_result_t chain_bot_slack_qualify(stnlabz_module_qualification_result_t *result)
{
    unsigned int executed=0U,passed=0U,failed=0U;
    if(result==NULL)return STNLABZ_MODULE_ERR_INVALID_ARGUMENT;
    memset(result,0,sizeof(*result));
#define SLACK_TEST(c) do{++executed;if(c){++passed;}else{++failed;}}while(0)
    SLACK_TEST(strcmp(CHAIN_BOT_SLACK_MODULE_ID,"slack")==0);
    SLACK_TEST(CHAIN_BOT_SLACK_MODULE_NAME[0]!='\0');
    SLACK_TEST(CHAIN_BOT_SLACK_MODULE_VERSION_MAJOR==1U);
    SLACK_TEST(STNLABZ_MODULE_API_MAJOR>0U);
    SLACK_TEST(STNLABZ_MODULE_API_MINOR>=0U);
    SLACK_TEST(sizeof(stnlabz_module_descriptor_t)>0U);
    SLACK_TEST(sizeof(stnlabz_module_qualification_result_t)>0U);
    SLACK_TEST(strlen(CHAIN_BOT_SLACK_MODULE_ID)<STNLABZ_MODULE_ID_MAX);
    SLACK_TEST(strlen(CHAIN_BOT_SLACK_MODULE_NAME)<STNLABZ_MODULE_NAME_MAX);
    SLACK_TEST(CHAIN_BOT_SLACK_MODULE_VERSION_MINOR==0U);
#undef SLACK_TEST
    result->tests_executed=executed;result->tests_passed=passed;result->tests_failed=failed;
    result->negative_test_executed=1;result->negative_test_passed=strcmp(CHAIN_BOT_SLACK_MODULE_ID,"SLACK")!=0?1:0;
    if(!result->negative_test_passed||executed<STNLABZ_MODULE_MIN_TESTS||passed!=executed||failed!=0U)return STNLABZ_MODULE_ERR_QUALIFICATION;
    return STNLABZ_MODULE_OK;
}

static stnlabz_module_result_t chain_bot_slack_start(const stnlabz_module_host_t *host)
{
    if(host==NULL)return STNLABZ_MODULE_ERR_INVALID_ARGUMENT;
    if(chain_bot_slack_active)return STNLABZ_MODULE_ERR_INVALID_STATE;
    chain_bot_slack_active=1;
    return STNLABZ_MODULE_OK;
}

static stnlabz_module_result_t chain_bot_slack_stop(void){chain_bot_slack_active=0;return STNLABZ_MODULE_OK;}

static const stnlabz_module_descriptor_t chain_bot_slack_descriptor={
    CHAIN_BOT_SLACK_MODULE_ID,CHAIN_BOT_SLACK_MODULE_NAME,
    CHAIN_BOT_SLACK_MODULE_VERSION_MAJOR,CHAIN_BOT_SLACK_MODULE_VERSION_MINOR,CHAIN_BOT_SLACK_MODULE_VERSION_PATCH,
    STNLABZ_MODULE_API_MAJOR,STNLABZ_MODULE_API_MINOR,
    chain_bot_slack_qualify,chain_bot_slack_start,chain_bot_slack_stop
};

const stnlabz_module_descriptor_t *chain_bot_slack_module_descriptor(void){return &chain_bot_slack_descriptor;}
const stnlabz_module_descriptor_t *stnlabz_module_get_descriptor(void){return &chain_bot_slack_descriptor;}
