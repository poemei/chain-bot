CC ?= cc
CFLAGS ?= -std=c11 -Wall -Wextra -Wpedantic -Werror -O2
CPPFLAGS := -Iincludes -Imodules/irc/includes -I../ABI/includes
LDLIBS := -lssl -lcrypto -ldl -lpthread
BUILD := build

ABI_SRC := ../ABI/src/abi.c ../ABI/src/module.c ../ABI/src/module_registry.c
IRC_TRANSPORT := modules/irc/src/chain_bot_irc_transport.c
IRC_SESSION := modules/irc/src/chain_bot_irc_session.c
IRC_RUNTIME := modules/irc/src/chain_bot_irc_runtime.c
IRC_LOOP := modules/irc/src/chain_bot_irc_loop.c
IRC_WORKER := modules/irc/src/chain_bot_irc_worker.c
IRC_MODULE := modules/irc/src/chain_bot_irc_module.c

TEST_BINS := \
 $(BUILD)/test_chain_bot \
 $(BUILD)/test_chain_bot_config \
 $(BUILD)/test_chain_bot_module \
 $(BUILD)/test_chain_bot_irc_runtime \
 $(BUILD)/test_chain_bot_irc_transport \
 $(BUILD)/test_chain_bot_irc_session \
 $(BUILD)/test_chain_bot_irc_loop \
 $(BUILD)/test_chain_bot_irc_worker \
 $(BUILD)/test_chain_bot_irc_module

.PHONY: all test clean check-abi

all: test

check-abi:
	@test -f ../ABI/includes/abi.h || (echo "ERROR: sibling ../ABI repository is required"; exit 1)

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/test_chain_bot: tests/test_chain_bot.c src/chain_bot.c | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@

$(BUILD)/test_chain_bot_config: tests/test_chain_bot_config.c src/chain_bot_config.c | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@

$(BUILD)/test_chain_bot_module: tests/test_chain_bot_module.c src/chain_bot_module.c $(ABI_SRC) | $(BUILD) check-abi
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@ $(LDLIBS)

$(BUILD)/test_chain_bot_irc_runtime: tests/test_chain_bot_irc_runtime.c $(IRC_RUNTIME) $(IRC_SESSION) $(IRC_TRANSPORT) | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@ $(LDLIBS)

$(BUILD)/test_chain_bot_irc_transport: modules/irc/tests/test_chain_bot_irc_transport.c $(IRC_TRANSPORT) | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@ $(LDLIBS)

$(BUILD)/test_chain_bot_irc_session: modules/irc/tests/test_chain_bot_irc_session.c $(IRC_SESSION) $(IRC_TRANSPORT) | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@ $(LDLIBS)

$(BUILD)/test_chain_bot_irc_loop: modules/irc/tests/test_chain_bot_irc_loop.c $(IRC_LOOP) $(IRC_RUNTIME) $(IRC_SESSION) $(IRC_TRANSPORT) | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@ $(LDLIBS)

$(BUILD)/test_chain_bot_irc_worker: modules/irc/tests/test_chain_bot_irc_worker.c $(IRC_WORKER) $(IRC_LOOP) $(IRC_RUNTIME) $(IRC_SESSION) $(IRC_TRANSPORT) | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@ $(LDLIBS)

$(BUILD)/test_chain_bot_irc_module: modules/irc/tests/test_chain_bot_irc_module.c $(IRC_MODULE) $(IRC_WORKER) $(IRC_LOOP) $(IRC_RUNTIME) $(IRC_SESSION) $(IRC_TRANSPORT) $(ABI_SRC) | $(BUILD) check-abi
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@ $(LDLIBS)

test: $(TEST_BINS)
	@set -e; \
	for test_bin in $(TEST_BINS); do \
		echo "==> $$test_bin"; \
		./$$test_bin; \
	done
	@echo "All Chain Bot tests passed."

clean:
	rm -rf $(BUILD)
