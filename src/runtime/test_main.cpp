#include "GlobalContext.h"
#include "hook.h"
#include "query.h"

#define MODULE "Init"
#include "log.h"

#include <stdio.h>

static hook_t hook_main2;

int our_main(int argc, char **argv) {
    LOGS("The hook hooked");
    LOG("we can print the program name: %s", argv[0]);
    SET_ORIGINAL(hook_main2, int, main2, int, char **);
    LOG("call orig: %p", (void *)orig_main2);
    orig_main2(argc, argv);
    return 0;
}

static hook_t hook_my_func2;

void my_func() {
    puts("{my_func}");
    SET_ORIGINAL(hook_my_func2, void, puts);
    orig_puts();
}

__attribute__((constructor)) void init_hooks() {
    disasm_t disasm;
    disasm_create(&disasm);
    {
        MakeNativeGuard ng;
        LOGS("before main native");
    }
    LOGS("before main non-native");

    void *main_addr = querySymbol(nullptr, "main");
    if (hook(&disasm, main_addr, 1024, (void *)our_main, &hook_main2)) {
        LOGS("Couldn't hook main :(");
    }


    if (hook(&disasm, querySymbol(nullptr, "my_func"), 1024, (void *)my_func, &hook_my_func2)) {
        LOGS("Couldn't hook my_func :(");
    }
    disasm_destroy(&disasm);
}
