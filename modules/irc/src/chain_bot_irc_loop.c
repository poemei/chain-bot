#include <stdio.h>
#include <string.h>

#include "chain_bot_irc_commands.h"
#include "chain_bot_irc_loop.h"

static void set_error(char *error, size_t error_size, const char *message)
{
    if (error != NULL && error_size > 0U) (void)snprintf(error, error_size, "%s", message);
}

static int handle_privmsg(chain_bot_irc_loop_t *loop,const char *line,char *error,size_t error_size)
{
    const char *bang,*command,*target_start,*target_end,*message;
    char sender[64],target[128],response[CHAIN_BOT_IRC_COMMAND_RESPONSE_MAX];
    size_t sender_len,target_len;

    if(line[0]!=':') return 1;
    bang=strchr(line,'!');
    if(bang==NULL) return 1;
    command=strstr(bang," PRIVMSG ");
    if(command==NULL) return 1;

    sender_len=(size_t)(bang-(line+1));
    if(sender_len==0U||sender_len>=sizeof(sender)) return 1;
    memcpy(sender,line+1,sender_len); sender[sender_len]='\0';

    target_start=command+9U;
    target_end=strchr(target_start,' ');
    if(target_end==NULL||target_end[1]!=':') return 1;
    target_len=(size_t)(target_end-target_start);
    if(target_len==0U||target_len>=sizeof(target)) return 1;
    memcpy(target,target_start,target_len); target[target_len]='\0';
    message=target_end+2U;

    if(message[0]!='!') return 1;

    if(strcmp(target,loop->runtime->session.channel)==0) {
        return chain_bot_irc_session_privmsg(
            &loop->runtime->session,
            "Please use /msg chain-bot !help for commands.",
            error,error_size);
    }

    if(strcmp(target,loop->runtime->session.nick)!=0) return 1;

    if(!chain_bot_irc_command_handle(message,loop->chain_config,response,sizeof(response),error,error_size)) {
        (void)snprintf(response,sizeof(response),"Unknown command. Use !help for available commands.");
    }

    return chain_bot_irc_session_privmsg_to(
        &loop->runtime->session,sender,response,error,error_size);
}

void chain_bot_irc_loop_init(chain_bot_irc_loop_t *loop,chain_bot_irc_runtime_t *runtime)
{
    if(loop==NULL)return;
    memset(loop,0,sizeof(*loop)); loop->runtime=runtime;
}

void chain_bot_irc_loop_set_chain_config(chain_bot_irc_loop_t *loop,const chain_bot_chain_config_t *chain_config)
{
    if(loop!=NULL) loop->chain_config=chain_config;
}

int chain_bot_irc_loop_feed(chain_bot_irc_loop_t *loop,const char *data,size_t length,char *error,size_t error_size)
{
    size_t index;
    if(loop==NULL||loop->runtime==NULL||(data==NULL&&length>0U)){set_error(error,error_size,"invalid IRC receive-loop input");return 0;}
    for(index=0U;index<length;++index){
        char byte=data[index];
        if(loop->pending_length>=sizeof(loop->pending)){loop->pending_length=0U;set_error(error,error_size,"IRC receive buffer exceeded");return 0;}
        loop->pending[loop->pending_length++]=byte;
        if(loop->pending_length>=2U&&loop->pending[loop->pending_length-2U]=='\r'&&loop->pending[loop->pending_length-1U]=='\n'){
            size_t line_length=loop->pending_length-2U; char line[CHAIN_BOT_IRC_LINE_MAX+1U];
            if(line_length==0U||line_length>CHAIN_BOT_IRC_LINE_MAX){loop->pending_length=0U;set_error(error,error_size,"invalid IRC frame length");return 0;}
            memcpy(line,loop->pending,line_length);line[line_length]='\0';loop->pending_length=0U;
            if(!chain_bot_irc_runtime_handle_line(loop->runtime,line,error,error_size))return 0;
            if(!handle_privmsg(loop,line,error,error_size))return 0;
        } else if(byte=='\n'){loop->pending_length=0U;set_error(error,error_size,"IRC frame missing CRLF terminator");return 0;}
    }
    if(loop->pending_length>CHAIN_BOT_IRC_LINE_MAX){loop->pending_length=0U;set_error(error,error_size,"IRC frame exceeds protocol limit");return 0;}
    return 1;
}

int chain_bot_irc_loop_receive_once(chain_bot_irc_loop_t *loop,char *error,size_t error_size)
{
    char buffer[1024];size_t received=0U;
    if(loop==NULL||loop->runtime==NULL){set_error(error,error_size,"invalid IRC receive-loop request");return 0;}
    if(!chain_bot_irc_runtime_receive(loop->runtime,buffer,sizeof(buffer),&received,error,error_size))return 0;
    if(received==0U){set_error(error,error_size,"IRC connection closed");return 0;}
    return chain_bot_irc_loop_feed(loop,buffer,received,error,error_size);
}
