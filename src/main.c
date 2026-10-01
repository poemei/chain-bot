#include <stdio.h>

#include "chain_bot_app.h"
#include "chain_bot_log.h"

int main(int argc, char **argv)
{
    chain_bot_app_t app;
    const char *config_path = CHAIN_BOT_CONFIG_DEFAULT_PATH;
    char error[256];
    char message[512];
    int run_result;

    if (argc > 2) {
        (void)fprintf(stderr, "Usage: chain-bot [config-path]\n");
        return 2;
    }

    if (argc == 2) {
        config_path = argv[1];
    }

    if (!chain_bot_log_open(CHAIN_BOT_LOG_DEFAULT_PATH, error, sizeof(error))) {
        (void)fprintf(stderr, "Chain Bot logging startup failed: %s\n", error);
        return 1;
    }

    chain_bot_log_info("Chain Bot starting.");
    chain_bot_app_init(&app);

    if (!chain_bot_app_start(&app, config_path, error, sizeof(error))) {
        (void)snprintf(message, sizeof(message), "Chain Bot startup failed: %s", error);
        chain_bot_log_error(message);
        chain_bot_app_stop(&app);
        chain_bot_log_close();
        return 1;
    }

    chain_bot_log_info("Chain Bot connected and operational.");
    run_result = chain_bot_app_run(&app);

    if (!run_result) {
        chain_bot_log_error("Chain Bot IRC worker stopped unexpectedly.");
    }

    chain_bot_log_info("Chain Bot stopping.");
    chain_bot_app_stop(&app);
    chain_bot_log_info("Chain Bot stopped.");
    chain_bot_log_close();
    return run_result ? 0 : 1;
}
