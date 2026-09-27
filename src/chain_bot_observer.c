#include <string.h>

#include "chain_bot_observer.h"

void chain_bot_observer_init(
    chain_bot_observer_t *observer,
    const chain_bot_chain_config_t *chain_config)
{
    if (observer == NULL) {
        return;
    }

    memset(observer, 0, sizeof(*observer));
    if (chain_config != NULL) {
        observer->chain_config = *chain_config;
    }
}

int chain_bot_observer_accept(
    chain_bot_observer_t *observer,
    const chain_bot_stnc_info_t *current,
    chain_bot_stnc_info_t *accepted,
    int *changed)
{
    int state_changed;

    if (observer == NULL || current == NULL || accepted == NULL || changed == NULL) {
        return 0;
    }

    *changed = 0;
    *accepted = *current;

    if (!observer->has_baseline) {
        observer->baseline = *current;
        observer->has_baseline = 1;
        return 1;
    }

    state_changed =
        current->height != observer->baseline.height ||
        memcmp(current->tip_id, observer->baseline.tip_id, sizeof(current->tip_id)) != 0;

    if (state_changed) {
        observer->baseline = *current;
        *changed = 1;
    }

    return 1;
}

int chain_bot_observer_poll(
    chain_bot_observer_t *observer,
    chain_bot_stnc_info_t *accepted,
    int *changed,
    char *error,
    size_t error_size)
{
    chain_bot_stnc_info_t current;

    if (observer == NULL || accepted == NULL || changed == NULL) {
        return 0;
    }

    if (!chain_bot_stnc_info_request(
            &observer->chain_config,
            &current,
            error,
            error_size)) {
        return 0;
    }

    return chain_bot_observer_accept(observer, &current, accepted, changed);
}
