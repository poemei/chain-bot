#include <stdio.h>
#include <string.h>

#include "chain_bot_stnc.h"

static void write32(unsigned char *p,unsigned long v){p[0]=(unsigned char)(v>>24);p[1]=(unsigned char)(v>>16);p[2]=(unsigned char)(v>>8);p[3]=(unsigned char)v;}
static void write64(unsigned char *p,unsigned long long v){int i;for(i=7;i>=0;--i){p[i]=(unsigned char)(v&255U);v>>=8;}}

static int test_decode_info(void)
{
    unsigned char frame[208]={0};chain_bot_stnc_info_t info;char error[128];
    memcpy(frame,"STNC",4);frame[5]=2;frame[7]=2;frame[9]=1;write32(frame+20,184);memset(frame+24,0x11,32);memset(frame+56,0x22,32);write64(frame+88,267);memset(frame+96,0x33,32);memset(frame+128,0x44,40);memset(frame+168,0x55,32);write32(frame+200,1);write32(frame+204,268);
    return chain_bot_stnc_decode_info(frame,sizeof(frame),&info,error,sizeof(error))&&info.height==267U&&info.protocol==1U&&info.block_count==268U&&info.tip_id[0]==0x33;
}

static int test_reject_bad_magic(void){unsigned char frame[208]={0};chain_bot_stnc_info_t info;char error[128];return !chain_bot_stnc_decode_info(frame,sizeof(frame),&info,error,sizeof(error));}
static int test_reject_bad_size(void){unsigned char frame[24]={0};chain_bot_stnc_info_t info;char error[128];return !chain_bot_stnc_decode_info(frame,sizeof(frame),&info,error,sizeof(error));}
static int test_request_rejects_null(void){chain_bot_stnc_info_t info;char error[128];return !chain_bot_stnc_info_request(NULL,&info,error,sizeof(error));}
static int test_request_rejects_empty_host(void){chain_bot_chain_config_t config;chain_bot_stnc_info_t info;char error[128];memset(&config,0,sizeof(config));config.port=18473;return !chain_bot_stnc_info_request(&config,&info,error,sizeof(error));}

int main(void){unsigned int run=0,pass=0;
#define T(x) do{++run;if(x()){++pass;printf("[PASS] %s\n",#x);}else printf("[FAIL] %s\n",#x);}while(0)
T(test_decode_info);T(test_reject_bad_magic);T(test_reject_bad_size);T(test_request_rejects_null);T(test_request_rejects_empty_host);
#undef T
printf("Chain Bot STNC tests: %u/%u passed.\n",pass,run);return pass==run?0:1;}
