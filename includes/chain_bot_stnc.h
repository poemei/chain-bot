#ifndef CHAIN_BOT_STNC_H
#define CHAIN_BOT_STNC_H

#include <stddef.h>
#include <stdint.h>

#include "chain_bot_config.h"

#define CHAIN_BOT_STNC_INFO_SIZE 184U

typedef struct chain_bot_stnc_info {
    uint8_t network_id[32];
    uint8_t genesis_id[32];
    uint64_t height;
    uint8_t tip_id[32];
    uint8_t cumulative_work[40];
    uint8_t current_target[32];
    uint32_t protocol;
    uint32_t block_count;
} chain_bot_stnc_info_t;

int chain_bot_stnc_info_request(
    const chain_bot_chain_config_t *config,
    chain_bot_stnc_info_t *info,
    char *error,
    size_t error_size);

int chain_bot_stnc_decode_info(
    const uint8_t *frame,
    size_t frame_size,
    chain_bot_stnc_info_t *info,
    char *error,
    size_t error_size);

#endif
