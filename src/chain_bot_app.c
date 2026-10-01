#define _POSIX_C_SOURCE 200809L

#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "chain_bot_app.h"
#include "chain_bot_log.h"

static volatile sig_atomic_t chain_bot_stop_requested = 0;

static void chain_bot_signal_handler(int signal_number)
{
    (void)signal_number;
    chain_bot_stop_requested = 1;
}

static void set_error(char *error, size_t error_size, const char *message)
{
    if (error != NULL && error_size > 0U) {
        (void)snprintf(error, error_size, "%s", message);
    }
}

static void log_module_event(const char *event,
                             const chain_bot_module_t *module)
{
    char message[256];

    if (event == NULL || module == NULL || module->descriptor == NULL) return;

    (void)snprintf(message, sizeof(message),
                   "Module %.32s: %.63s v%u.%u.%u | %.127s",
                   event,
                   module->descriptor->id,
                   module->descriptor->version_major,
                   module->descriptor->version_minor,
                   module->descriptor->version_patch,
                   module->path);
    chain_bot_log_info(message);
}

static int load_module(chain_bot_app_t *app,
                       chain_bot_module_t *module,
                       const char *path,
                       int required,
                       char *error,
                       size_t error_size)
{
    chain_bot_module_result_t result;
    char message[512];

    result = chain_bot_module_load(module, path, &app->module_registry);
    if (result != CHAIN_BOT_MODULE_OK) {
        (void)snprintf(message, sizeof(message),
                       "Module hot-load failed: %.383s | %.95s",
                       path, chain_bot_module_result_string(result));
        if (required) {
            chain_bot_log_error(message);
            set_error(error, error_size, message);
            return 0;
        }
        chain_bot_log_error(message);
        return 1;
    }

    log_module_event("discovered, verified and qualified", module);
    log_module_event("hot-loaded", module);

    result = chain_bot_module_activate(module, &app->module_registry, &app->module_host);
    if (result != CHAIN_BOT_MODULE_OK) {
        (void)snprintf(message, sizeof(message),
                       "Module activation failed: %.63s | %.95s",
                       module->descriptor->id,
                       chain_bot_module_result_string(result));
        chain_bot_log_error(message);
        if (required) {
            set_error(error, error_size, message);
            return 0;
        }
        return 1;
    }

    log_module_event("active", module);
    ++app->active_modules;
    return 1;
}

static void hex_encode(const unsigned char *input, size_t length,
                       char *output, size_t output_size)
{
    static const char hex[] = "0123456789abcdef";
    size_t i;

    if (input == NULL || output == NULL || output_size < (length * 2U) + 1U) return;

    for (i = 0U; i < length; ++i) {
        output[i * 2U] = hex[(input[i] >> 4) & 0x0fU];
        output[(i * 2U) + 1U] = hex[input[i] & 0x0fU];
    }
    output[length * 2U] = '\0';
}

void chain_bot_app_init(chain_bot_app_t *app)
{
    if (app == NULL) return;
    memset(app, 0, sizeof(*app));
    stnlabz_module_registry_init(&app->module_registry);
    memset(&app->module_host, 0, sizeof(app->module_host));
    chain_bot_module_init(&app->irc_module);
    chain_bot_module_init(&app->slack_module);
    chain_bot_irc_runtime_init(&app->irc_runtime);
    chain_bot_irc_worker_init(&app->irc_worker, &app->irc_runtime);
    chain_bot_observer_init(&app->observer, NULL);
}

int chain_bot_app_start(chain_bot_app_t *app, const char *config_path,
                        char *error, size_t error_size)
{
    chain_bot_stnc_info_t baseline;
    char message[256];
    int changed;

    if (app == NULL || config_path == NULL || config_path[0] == '\0') {
        set_error(error, error_size, "invalid Chain Bot startup request");
        return 0;
    }
    if (app->running) {
        set_error(error, error_size, "Chain Bot is already running");
        return 0;
    }
    if (!chain_bot_config_load(config_path, &app->config, error, error_size)) return 0;

    chain_bot_log_info("Module discovery started.");
    if (!load_module(app, &app->irc_module, CHAIN_BOT_IRC_MODULE_PATH,
                     1, error, error_size)) return 0;
    if (!load_module(app, &app->slack_module, CHAIN_BOT_SLACK_MODULE_PATH,
                     0, error, error_size)) {
        (void)chain_bot_module_unload(&app->irc_module, &app->module_registry);
        return 0;
    }

    if (!chain_bot_irc_runtime_connect(&app->irc_runtime, &app->config.irc,
                                       error, error_size)) {
        (void)chain_bot_module_unload(&app->slack_module, &app->module_registry);
        (void)chain_bot_module_unload(&app->irc_module, &app->module_registry);
        return 0;
    }
    chain_bot_log_info("IRC runtime connected.");

    chain_bot_observer_init(&app->observer, &app->config.chain);
    if (!chain_bot_observer_poll(&app->observer, &baseline, &changed,
                                 error, error_size)) {
        chain_bot_irc_runtime_close(&app->irc_runtime);
        (void)chain_bot_module_unload(&app->slack_module, &app->module_registry);
        (void)chain_bot_module_unload(&app->irc_module, &app->module_registry);
        return 0;
    }

    (void)snprintf(message, sizeof(message),
                   "Chain observer connected: %.127s:%u | accepted height %llu",
                   app->config.chain.host,
                   app->config.chain.port,
                   (unsigned long long)baseline.height);
    chain_bot_log_info(message);

    chain_bot_irc_worker_init(&app->irc_worker, &app->irc_runtime);
    chain_bot_irc_loop_set_chain_config(&app->irc_worker.loop, &app->config.chain);
    if (!chain_bot_irc_worker_start(&app->irc_worker)) {
        chain_bot_irc_runtime_close(&app->irc_runtime);
        (void)chain_bot_module_unload(&app->slack_module, &app->module_registry);
        (void)chain_bot_module_unload(&app->irc_module, &app->module_registry);
        set_error(error, error_size, "unable to start IRC receive worker");
        return 0;
    }

    (void)snprintf(message, sizeof(message),
                   "Chain Bot module state: %u active module%s.",
                   app->active_modules,
                   app->active_modules == 1U ? "" : "s");
    chain_bot_log_info(message);

    app->running = 1;
    return 1;
}

int chain_bot_app_run(chain_bot_app_t *app)
{
    struct sigaction action;
    struct timespec delay;
    unsigned int ticks = 0U;
    char error[256];

    if (app == NULL || !app->running) return 0;

    memset(&action, 0, sizeof(action));
    action.sa_handler = chain_bot_signal_handler;
    sigemptyset(&action.sa_mask);
    (void)sigaction(SIGINT, &action, NULL);
    (void)sigaction(SIGTERM, &action, NULL);

    chain_bot_stop_requested = 0;
    delay.tv_sec = 0;
    delay.tv_nsec = 100000000L;

    while (!chain_bot_stop_requested && !chain_bot_irc_worker_failed(&app->irc_worker)) {
        chain_bot_stnc_info_t previous;
        chain_bot_stnc_info_t accepted;
        int changed = 0;

        (void)nanosleep(&delay, NULL);
        ++ticks;
        if (ticks < 50U) continue;
        ticks = 0U;

        previous = app->observer.baseline;
        if (!chain_bot_observer_poll(&app->observer, &accepted, &changed,
                                     error, sizeof(error))) continue;

        if (changed) {
            char tip[65];
            char message[256];

            hex_encode(accepted.tip_id, sizeof(accepted.tip_id), tip, sizeof(tip));
            if (accepted.height > previous.height) {
                (void)snprintf(message, sizeof(message),
                               "[STNC] New accepted block - height %llu | tip %s",
                               (unsigned long long)accepted.height, tip);
            } else {
                (void)snprintf(message, sizeof(message),
                               "[STNC] Accepted tip changed - height %llu | tip %s",
                               (unsigned long long)accepted.height, tip);
            }

            (void)chain_bot_irc_runtime_announce_with_slack(
                &app->irc_runtime, &app->config.slack,
                message, error, sizeof(error));
        }
    }

    return chain_bot_irc_worker_failed(&app->irc_worker) ? 0 : 1;
}

void chain_bot_app_stop(chain_bot_app_t *app)
{
    if (app == NULL) return;

    if (app->running) {
        chain_bot_irc_worker_stop(&app->irc_worker);
        chain_bot_irc_runtime_close(&app->irc_runtime);
    }

    if (app->slack_module.handle != NULL) {
        log_module_event("unloading", &app->slack_module);
        if (chain_bot_module_unload(&app->slack_module,
                                    &app->module_registry) == CHAIN_BOT_MODULE_OK) {
            chain_bot_log_info("Module unloaded: slack");
        }
    }

    if (app->irc_module.handle != NULL) {
        log_module_event("unloading", &app->irc_module);
        if (chain_bot_module_unload(&app->irc_module,
                                    &app->module_registry) == CHAIN_BOT_MODULE_OK) {
            chain_bot_log_info("Module unloaded: irc");
        }
    }

    app->active_modules = 0U;
    app->running = 0;
}
