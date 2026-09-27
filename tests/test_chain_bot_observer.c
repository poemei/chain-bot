#include <stdio.h>
#include <string.h>

#include "chain_bot_observer.h"

static chain_bot_stnc_info_t make_info(unsigned long long height, unsigned char tip_byte)
{
    chain_bot_stnc_info_t info;
    memset(&info, 0, sizeof(info));
    info.height = (uint64_t)height;
    info.block_count = (uint32_t)(height + 1ULL);
    memset(info.tip_id, tip_byte, sizeof(info.tip_id));
    return info;
}

static int test_first_state_is_baseline_only(void)
{
    chain_bot_observer_t observer;
    chain_bot_stnc_info_t current = make_info(399ULL, 0x11U);
    chain_bot_stnc_info_t accepted;
    int changed = -1;

    chain_bot_observer_init(&observer, NULL);
    return chain_bot_observer_accept(&observer, &current, &accepted, &changed) &&
           observer.has_baseline && changed == 0 && observer.baseline.height == 399ULL;
}

static int test_unchanged_state_is_quiet(void)
{
    chain_bot_observer_t observer;
    chain_bot_stnc_info_t current = make_info(399ULL, 0x11U);
    chain_bot_stnc_info_t accepted;
    int changed = -1;

    chain_bot_observer_init(&observer, NULL);
    (void)chain_bot_observer_accept(&observer, &current, &accepted, &changed);
    changed = -1;
    return chain_bot_observer_accept(&observer, &current, &accepted, &changed) && changed == 0;
}

static int test_new_height_announces_once(void)
{
    chain_bot_observer_t observer;
    chain_bot_stnc_info_t first = make_info(399ULL, 0x11U);
    chain_bot_stnc_info_t next = make_info(400ULL, 0x22U);
    chain_bot_stnc_info_t accepted;
    int changed = 0;

    chain_bot_observer_init(&observer, NULL);
    (void)chain_bot_observer_accept(&observer, &first, &accepted, &changed);
    if (!chain_bot_observer_accept(&observer, &next, &accepted, &changed) || !changed) {
        return 0;
    }
    changed = -1;
    return chain_bot_observer_accept(&observer, &next, &accepted, &changed) && changed == 0;
}

static int test_tip_change_same_height_is_event(void)
{
    chain_bot_observer_t observer;
    chain_bot_stnc_info_t first = make_info(400ULL, 0x22U);
    chain_bot_stnc_info_t reorg = make_info(400ULL, 0x33U);
    chain_bot_stnc_info_t accepted;
    int changed = 0;

    chain_bot_observer_init(&observer, NULL);
    (void)chain_bot_observer_accept(&observer, &first, &accepted, &changed);
    return chain_bot_observer_accept(&observer, &reorg, &accepted, &changed) && changed == 1;
}

static int test_invalid_arguments(void)
{
    chain_bot_observer_t observer;
    chain_bot_stnc_info_t info = make_info(1ULL, 0x01U);
    int changed = 0;

    chain_bot_observer_init(&observer, NULL);
    return !chain_bot_observer_accept(NULL, &info, &info, &changed) &&
           !chain_bot_observer_accept(&observer, NULL, &info, &changed) &&
           !chain_bot_observer_accept(&observer, &info, NULL, &changed) &&
           !chain_bot_observer_accept(&observer, &info, &info, NULL);
}

int main(void)
{
    unsigned int executed = 0U;
    unsigned int passed = 0U;

#define RUN_TEST(test_fn) do { ++executed; if ((test_fn)()) { ++passed; printf("[PASS] %s\n", #test_fn); } else { printf("[FAIL] %s\n", #test_fn); } } while (0)
    RUN_TEST(test_first_state_is_baseline_only);
    RUN_TEST(test_unchanged_state_is_quiet);
    RUN_TEST(test_new_height_announces_once);
    RUN_TEST(test_tip_change_same_height_is_event);
    RUN_TEST(test_invalid_arguments);
#undef RUN_TEST

    printf("Chain Bot observer tests: %u/%u passed.\n", passed, executed);
    return passed == executed ? 0 : 1;
}
