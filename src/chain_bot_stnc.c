#define _POSIX_C_SOURCE 200112L

#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "chain_bot_stnc.h"

#define STNC_HEADER_SIZE 24U
#define STNC_INFO_FRAME_SIZE (STNC_HEADER_SIZE + CHAIN_BOT_STNC_INFO_SIZE)

static void set_error(char *error,size_t size,const char *message){if(error!=NULL&&size>0U)(void)snprintf(error,size,"%s",message);}
static uint16_t read16(const uint8_t *p){return (uint16_t)(((uint16_t)p[0]<<8)|(uint16_t)p[1]);}
static uint32_t read32(const uint8_t *p){return ((uint32_t)p[0]<<24)|((uint32_t)p[1]<<16)|((uint32_t)p[2]<<8)|(uint32_t)p[3];}
static uint64_t read64(const uint8_t *p){uint64_t v=0;size_t i;for(i=0;i<8U;++i)v=(v<<8)|p[i];return v;}
static void write64(uint8_t *p,uint64_t v){int i;for(i=7;i>=0;--i){p[i]=(uint8_t)(v&0xffU);v>>=8;}}

int chain_bot_stnc_decode_info(const uint8_t *frame,size_t frame_size,chain_bot_stnc_info_t *info,char *error,size_t error_size)
{
    const uint8_t *p;
    if(frame==NULL||info==NULL||frame_size!=STNC_INFO_FRAME_SIZE){set_error(error,error_size,"invalid STNC INFO frame size");return 0;}
    if(memcmp(frame,"STNC",4)!=0||read16(frame+4)!=2U||read16(frame+6)!=2U||read16(frame+8)!=1U||read16(frame+10)!=0U||read32(frame+20)!=CHAIN_BOT_STNC_INFO_SIZE){set_error(error,error_size,"invalid STNC INFO response");return 0;}
    p=frame+STNC_HEADER_SIZE;
    memcpy(info->network_id,p,32);memcpy(info->genesis_id,p+32,32);info->height=read64(p+64);memcpy(info->tip_id,p+72,32);memcpy(info->cumulative_work,p+104,40);memcpy(info->current_target,p+144,32);info->protocol=read32(p+176);info->block_count=read32(p+180);
    return 1;
}

int chain_bot_stnc_info_request(const chain_bot_chain_config_t *config,chain_bot_stnc_info_t *info,char *error,size_t error_size)
{
    struct addrinfo hints,*addresses=NULL,*a;char service[6];int fd=-1,status;uint8_t request[STNC_HEADER_SIZE]={0};uint8_t response[STNC_INFO_FRAME_SIZE];size_t at=0;
    if(config==NULL||info==NULL||config->host[0]=='\0'||config->port==0U){set_error(error,error_size,"invalid STNC INFO request");return 0;}
    memset(&hints,0,sizeof(hints));hints.ai_family=AF_UNSPEC;hints.ai_socktype=SOCK_STREAM;hints.ai_protocol=IPPROTO_TCP;(void)snprintf(service,sizeof(service),"%u",(unsigned)config->port);
    status=getaddrinfo(config->host,service,&hints,&addresses);if(status!=0){set_error(error,error_size,"unable to resolve Chain RPC host");return 0;}
    for(a=addresses;a!=NULL;a=a->ai_next){fd=socket(a->ai_family,a->ai_socktype,a->ai_protocol);if(fd>=0&&connect(fd,a->ai_addr,a->ai_addrlen)==0)break;if(fd>=0)(void)close(fd);fd=-1;}freeaddrinfo(addresses);if(fd<0){set_error(error,error_size,"unable to connect to Chain RPC");return 0;}
    memcpy(request,"STNC",4);request[5]=2;request[7]=1;request[9]=1;write64(request+12,1U);
    while(at<sizeof(request)){ssize_t n=send(fd,request+at,sizeof(request)-at,0);if(n<=0){(void)close(fd);set_error(error,error_size,"unable to send STNC INFO request");return 0;}at+=(size_t)n;}
    at=0;while(at<sizeof(response)){ssize_t n=recv(fd,response+at,sizeof(response)-at,0);if(n<=0){(void)close(fd);set_error(error,error_size,"incomplete STNC INFO response");return 0;}at+=(size_t)n;}(void)close(fd);
    return chain_bot_stnc_decode_info(response,sizeof(response),info,error,error_size);
}
