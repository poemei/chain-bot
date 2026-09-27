#ifndef CHAIN_BOT_OBSERVER_H
#define CHAIN_BOT_OBSERVER_H

#include <stddef.h>
#include <stdint.h>

#include "chain_bot_config.h"
#include "chain_bot_stnc.h"

typedef struct chain_bot_observer {
    chain_bot_chain_config_t chain_config;
    chain_bot_stnc_info_t baseline;
    int has_baseline;
} chain_bot_observer_t;

void chain_bot_observer_init(
    chain_bot_observer_t *observer,
    const chain_bot_chain_config_t *chain_config);

int chain_bot_observer_poll(
    chain_bot_observer_t *observer,
    chain_bot_stnc_info_t *accepted,
    int *changed,
    char *error,
    size_t error_size);

int chain_bot_observer_accept(
    chain_bot_observer_t *observer,
    const chain_bot_stnc_info_t *current,
    chain_bot_stnc_info_t *accepted,
    int *changed);

#endif
