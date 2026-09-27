#define _POSIX_C_SOURCE 200809L

#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "chain_bot_app.h"

static volatile sig_atomic_t chain_bot_stop_requested = 0;

static void chain_bot_signal_handler(int signal_number){(void)signal_number;chain_bot_stop_requested=1;}
static void set_error(char *error,size_t error_size,const char *message){if(error!=NULL&&error_size>0U)(void)snprintf(error,error_size,"%s",message);}

void chain_bot_app_init(chain_bot_app_t *app)
{
    if(app==NULL)return;
    memset(app,0,sizeof(*app));
    chain_bot_irc_runtime_init(&app->irc_runtime);
    chain_bot_irc_worker_init(&app->irc_worker,&app->irc_runtime);
}

int chain_bot_app_start(chain_bot_app_t *app,const char *config_path,char *error,size_t error_size)
{
    if(app==NULL||config_path==NULL||config_path[0]=='\0'){set_error(error,error_size,"invalid Chain Bot startup request");return 0;}
    if(app->running){set_error(error,error_size,"Chain Bot is already running");return 0;}
    if(!chain_bot_config_load(config_path,&app->config,error,error_size))return 0;
    if(!chain_bot_irc_runtime_connect(&app->irc_runtime,&app->config.irc,error,error_size))return 0;
    chain_bot_irc_worker_init(&app->irc_worker,&app->irc_runtime);
    chain_bot_irc_loop_set_chain_config(&app->irc_worker.loop,&app->config.chain);
    if(!chain_bot_irc_worker_start(&app->irc_worker)){
        chain_bot_irc_runtime_close(&app->irc_runtime);set_error(error,error_size,"unable to start IRC receive worker");return 0;
    }
    app->running=1;return 1;
}

int chain_bot_app_run(chain_bot_app_t *app)
{
    struct sigaction action;struct timespec delay;
    if(app==NULL||!app->running)return 0;
    memset(&action,0,sizeof(action));action.sa_handler=chain_bot_signal_handler;sigemptyset(&action.sa_mask);
    (void)sigaction(SIGINT,&action,NULL);(void)sigaction(SIGTERM,&action,NULL);
    chain_bot_stop_requested=0;delay.tv_sec=0;delay.tv_nsec=100000000L;
    while(!chain_bot_stop_requested&&!chain_bot_irc_worker_failed(&app->irc_worker))(void)nanosleep(&delay,NULL);
    return chain_bot_irc_worker_failed(&app->irc_worker)?0:1;
}

void chain_bot_app_stop(chain_bot_app_t *app)
{
    if(app==NULL)return;
    if(app->running){chain_bot_irc_worker_stop(&app->irc_worker);chain_bot_irc_runtime_close(&app->irc_runtime);}
    app->running=0;
}
