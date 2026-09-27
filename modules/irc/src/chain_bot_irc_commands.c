#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "chain_bot_irc_commands.h"
#include "chain_bot_stnc.h"

static void hex_encode(const uint8_t *input, size_t length, char *output, size_t output_size)
{
    static const char hex[] = "0123456789abcdef";
    size_t i;

    if (output == NULL || output_size < (length * 2U) + 1U) {
        return;
    }

    for (i = 0U; i < length; ++i) {
        output[i * 2U] = hex[(input[i] >> 4) & 0x0fU];
        output[(i * 2U) + 1U] = hex[input[i] & 0x0fU];
    }
    output[length * 2U] = '\0';
}

static int write_response(char *response, size_t response_size, const char *text)
{
    int count;

    if (response == NULL || response_size == 0U || text == NULL) {
        return 0;
    }

    count = snprintf(response, response_size, "%s", text);
    return count >= 0 && (size_t)count < response_size;
}

int chain_bot_irc_command_handle(
    const char *message,
    const chain_bot_chain_config_t *chain_config,
    char *response,
    size_t response_size,
    char *error,
    size_t error_size)
{
    chain_bot_stnc_info_t info;
    char tip[65];
    char network[65];
    char genesis[65];
    int count;

    if (message == NULL || response == NULL || response_size == 0U) {
        return 0;
    }

    response[0] = '\0';

    if (strcmp(message, "!help") == 0) {
        return write_response(response, response_size,
            "Public commands: !status !height !tip !network !block !about !whitepaper !website !bot");
    }

    if (strcmp(message, "!about") == 0) {
        return write_response(response, response_size,
            "STN Chain is a deterministic consensus network for verifiable records, agreements, distributed state, and independent data exchange.");
    }

    if (strcmp(message, "!whitepaper") == 0) {
        return write_response(response, response_size,
            "STN Chain whitepaper: https://stn-chain.org/docs/whitepaper");
    }

    if (strcmp(message, "!website") == 0) {
        return write_response(response, response_size, "STN Chain: https://stn-chain.org");
    }

    if (strcmp(message, "!bot") == 0) {
        return write_response(response, response_size,
            "Chain Bot: Linux ISO C observer for STN Chain and #STNC-Chain.");
    }

    if (strcmp(message, "!status") != 0 && strcmp(message, "!height") != 0 &&
        strcmp(message, "!tip") != 0 && strcmp(message, "!network") != 0 &&
        strcmp(message, "!block") != 0) {
        return 0;
    }

    if (chain_config == NULL ||
        !chain_bot_stnc_info_request(chain_config, &info, error, error_size)) {
        return write_response(response, response_size,
            "STN Chain information is temporarily unavailable.");
    }

    hex_encode(info.tip_id, sizeof(info.tip_id), tip, sizeof(tip));
    hex_encode(info.network_id, sizeof(info.network_id), network, sizeof(network));
    hex_encode(info.genesis_id, sizeof(info.genesis_id), genesis, sizeof(genesis));

    if (strcmp(message, "!height") == 0) {
        count = snprintf(response, response_size,
            "STN Chain accepted height: %llu", (unsigned long long)info.height);
    } else if (strcmp(message, "!tip") == 0) {
        count = snprintf(response, response_size,
            "STN Chain accepted tip: %s", tip);
    } else if (strcmp(message, "!network") == 0) {
        count = snprintf(response, response_size,
            "STN Chain network: %s | genesis: %s", network, genesis);
    } else if (strcmp(message, "!block") == 0) {
        count = snprintf(response, response_size,
            "Latest accepted block: height %llu | tip %s",
            (unsigned long long)info.height, tip);
    } else {
        count = snprintf(response, response_size,
            "STN Chain: height %llu | blocks %u | protocol %u | connected",
            (unsigned long long)info.height, info.block_count, info.protocol);
    }

    return count >= 0 && (size_t)count < response_size;
}
