#include <stddef.h>
#include <string.h>

#include "chain_bot_irc_worker.h"

static void *chain_bot_irc_worker_main(void *context)
{
    chain_bot_irc_worker_t *worker = (chain_bot_irc_worker_t *)context;
    char error[256];

    if (worker == NULL) {
        return NULL;
    }

    while (!worker->stop_requested) {
        if (!chain_bot_irc_loop_receive_once(
                &worker->loop,
                error,
                sizeof(error))) {
            if (!worker->stop_requested) {
                worker->failed = 1;
            }
            break;
        }
    }

    worker->running = 0;
    return NULL;
}

void chain_bot_irc_worker_init(
    chain_bot_irc_worker_t *worker,
    chain_bot_irc_runtime_t *runtime)
{
    if (worker == NULL) {
        return;
    }

    memset(worker, 0, sizeof(*worker));
    chain_bot_irc_loop_init(&worker->loop, runtime);
}

int chain_bot_irc_worker_start(chain_bot_irc_worker_t *worker)
{
    if (worker == NULL || worker->loop.runtime == NULL ||
        !worker->loop.runtime->connected || worker->running) {
        return 0;
    }

    worker->stop_requested = 0;
    worker->failed = 0;
    worker->running = 1;

    if (pthread_create(
            &worker->thread,
            NULL,
            chain_bot_irc_worker_main,
            worker) != 0) {
        worker->running = 0;
        return 0;
    }

    return 1;
}

void chain_bot_irc_worker_stop(chain_bot_irc_worker_t *worker)
{
    if (worker == NULL) {
        return;
    }

    worker->stop_requested = 1;

    if (worker->running) {
        if (worker->loop.runtime != NULL) {
            chain_bot_irc_runtime_close(worker->loop.runtime);
        }
        (void)pthread_join(worker->thread, NULL);
    }

    worker->running = 0;
}

int chain_bot_irc_worker_failed(const chain_bot_irc_worker_t *worker)
{
    return worker != NULL ? worker->failed : 1;
}
