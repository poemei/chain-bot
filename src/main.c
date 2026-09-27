#include <stdio.h>

#include "chain_bot_app.h"

int main(int argc, char **argv)
{
    chain_bot_app_t app;
    const char *config_path = CHAIN_BOT_CONFIG_DEFAULT_PATH;
    char error[256];
    int run_result;

    if (argc > 2) {
        (void)fprintf(stderr, "Usage: chain-bot [config-path]\n");
        return 2;
    }

    if (argc == 2) {
        config_path = argv[1];
    }

    chain_bot_app_init(&app);

    if (!chain_bot_app_start(&app, config_path, error, sizeof(error))) {
        (void)fprintf(stderr, "Chain Bot startup failed: %s\n", error);
        chain_bot_app_stop(&app);
        return 1;
    }

    (void)printf("Chain Bot connected.\n");
    run_result = chain_bot_app_run(&app);

    if (!run_result) {
        (void)fprintf(stderr, "Chain Bot IRC worker stopped unexpectedly.\n");
    }

    chain_bot_app_stop(&app);
    return run_result ? 0 : 1;
}
