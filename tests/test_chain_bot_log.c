#include <stdio.h>
#include <string.h>

#include "chain_bot_log.h"

static int file_contains(const char *path, const char *needle)
{
    FILE *file;
    char buffer[1024];
    size_t read_count;

    file = fopen(path, "r");
    if (file == NULL) return 0;

    read_count = fread(buffer, 1U, sizeof(buffer) - 1U, file);
    buffer[read_count] = '\0';
    (void)fclose(file);

    return strstr(buffer, needle) != NULL;
}

int main(void)
{
    const char *path = "build/test_chain_bot.log";
    char error[256];
    unsigned int executed = 0U;
    unsigned int passed = 0U;

#define TEST(c) do { ++executed; if (c) { ++passed; printf("[PASS] %s\n", #c); } else { printf("[FAIL] %s\n", #c); } } while (0)

    (void)remove(path);

    TEST(!chain_bot_log_open(NULL, error, sizeof(error)));
    TEST(!chain_bot_log_open("", error, sizeof(error)));
    TEST(chain_bot_log_open(path, error, sizeof(error)));

    chain_bot_log_info("test information message");
    chain_bot_log_error("test error message");
    chain_bot_log_close();

    TEST(file_contains(path, "[INFO] test information message"));
    TEST(file_contains(path, "[ERROR] test error message"));

    (void)remove(path);

#undef TEST

    printf("Chain Bot logging tests: %u/%u passed.\n", passed, executed);
    return passed == executed ? 0 : 1;
}
