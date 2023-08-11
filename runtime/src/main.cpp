#include "GlobalContext.h"
#include "Hook.h"
#include "query.h"

static Hook hook_main;

int our_main(int argc, char **argv) {
    printf("Wee the hook hooked!\n");
    printf("we can print the program name: %s\n", argv[0]);
    int (*orig)(int, char **) = (int (*)(int, char **))hook_main.original();
    printf("call orig: %p\n", orig);
    orig(argc, argv);
    return 0;
}

static Hook hook_my_func;

void my_func() {
    puts("[my_func]");
    void (*orig)() = (void (*)())hook_my_func.original();
    orig();
}

__attribute__((constructor)) void init_hooks() {
    {
        MakeNativeGuard ng;
        printf("before main native\n");
    }
    printf("before main non-native\n");

    void *main_addr = query_symbol(nullptr, "main");
    hook_main.set_destination((void *)our_main);
    hook_main.set_source(main_addr);
    if (!hook_main.hook()) {
        printf("Couldn't hook :(\n");
    }

    hook_my_func.set_destination((void *)my_func);
    hook_my_func.set_source(query_symbol(nullptr, "my_func"));
    if (!hook_my_func.hook()) {
        printf("Couldn't hook 2:(\n");
    }
}
