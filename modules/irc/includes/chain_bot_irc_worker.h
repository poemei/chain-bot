#ifndef CHAIN_BOT_IRC_WORKER_H
#define CHAIN_BOT_IRC_WORKER_H

#include <pthread.h>

#include "chain_bot_irc_loop.h"

typedef struct chain_bot_irc_worker {
    pthread_t thread;
    chain_bot_irc_loop_t loop;
    int running;
    int stop_requested;
    int failed;
} chain_bot_irc_worker_t;

void chain_bot_irc_worker_init(
    chain_bot_irc_worker_t *worker,
    chain_bot_irc_runtime_t *runtime);

int chain_bot_irc_worker_start(chain_bot_irc_worker_t *worker);

void chain_bot_irc_worker_stop(chain_bot_irc_worker_t *worker);

int chain_bot_irc_worker_failed(const chain_bot_irc_worker_t *worker);

#endif
