#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <string.h>
#include <curl/curl.h>
#include "chain_bot_slack.h"

static void set_error(char *out,size_t size,const char *text){if(out!=NULL&&size>0U)(void)snprintf(out,size,"%s",text);}

static int escape_json(const char *in,char *out,size_t size){size_t n=0U;if(in==NULL||out==NULL||size==0U)return 0;while(*in!='\0'){const char *s;char one[2];switch(*in){case '"':s="\\\"";break;case '\\':s="\\\\";break;case '\n':s="\\n";break;case '\r':s="\\r";break;case '\t':s="\\t";break;default:one[0]=*in;one[1]='\0';s=one;break;}if(n+strlen(s)+1U>size)return 0;memcpy(out+n,s,strlen(s));n+=strlen(s);++in;}out[n]='\0';return 1;}

int chain_bot_slack_announce(const chain_bot_slack_config_t *config,const char *message,char *error,size_t error_size)
{
    CURL *curl;CURLcode result;struct curl_slist *headers=NULL;char escaped[1024];char payload[1100];long status=0L;
    if(config==NULL||message==NULL||message[0]=='\0'){set_error(error,error_size,"invalid Slack announcement request");return 0;}
    if(!config->enabled)return 1;
    if(strncmp(config->webhook,"https://",8U)!=0){set_error(error,error_size,"Slack webhook must use HTTPS");return 0;}
    if(!escape_json(message,escaped,sizeof(escaped))||snprintf(payload,sizeof(payload),"{\"text\":\"%s\"}",escaped)<0){set_error(error,error_size,"unable to encode Slack announcement");return 0;}
    curl=curl_easy_init();if(curl==NULL){set_error(error,error_size,"unable to initialize Slack HTTPS transport");return 0;}
    headers=curl_slist_append(headers,"Content-Type: application/json");if(headers==NULL){curl_easy_cleanup(curl);set_error(error,error_size,"unable to initialize Slack HTTP headers");return 0;}
    (void)curl_easy_setopt(curl,CURLOPT_URL,config->webhook);(void)curl_easy_setopt(curl,CURLOPT_HTTPHEADER,headers);(void)curl_easy_setopt(curl,CURLOPT_POSTFIELDS,payload);(void)curl_easy_setopt(curl,CURLOPT_TIMEOUT,10L);(void)curl_easy_setopt(curl,CURLOPT_CONNECTTIMEOUT,5L);(void)curl_easy_setopt(curl,CURLOPT_SSL_VERIFYPEER,1L);(void)curl_easy_setopt(curl,CURLOPT_SSL_VERIFYHOST,2L);(void)curl_easy_setopt(curl,CURLOPT_NOSIGNAL,1L);
    result=curl_easy_perform(curl);if(result==CURLE_OK)(void)curl_easy_getinfo(curl,CURLINFO_RESPONSE_CODE,&status);curl_slist_free_all(headers);curl_easy_cleanup(curl);
    if(result!=CURLE_OK){set_error(error,error_size,"Slack webhook HTTPS request failed");return 0;}if(status<200L||status>=300L){set_error(error,error_size,"Slack webhook rejected announcement");return 0;}return 1;
}
