#include "GlobalContext.h"
#include "Hook.h"
#include "asm/GlobalLLVM.h"
#include "query.h"

#include <stdio.h>

static Hook hook_main;

int our_main(int argc, char **argv) {
    printf("[TestMain] Wee the hook hooked!\n");
    printf("[TestMain] we can print the program name: %s\n", argv[0]);
    ORIGINAL(hook_main, int, main, int, char **);
    printf("[TestMain] call orig: %p\n", orig_main);
    orig_main(argc, argv);
    return 0;
}

static Hook hook_my_func;

void my_func() {
    puts("{my_func}");
    ORIGINAL(hook_my_func, void, puts);
    orig_puts();
}

__attribute__((constructor)) void init_hooks() {
    {
        MakeNativeGuard ng;
        printf("[init] before main native\n");
    }
    printf("[init] before main non-native\n");

    void *main_addr = querySymbol(nullptr, "main");
    hook_main.setDestination((void *)our_main);
    hook_main.setSource(main_addr);
    if (!hook_main.hook()) {
        printf("[init] Couldn't hook main :(\n");
    }

    hook_my_func.setDestination((void *)my_func);
    hook_my_func.setSource(querySymbol(nullptr, "my_func"));
    if (!hook_my_func.hook()) {
        printf("[init] Couldn't hook my_func :(\n");
    }
}
