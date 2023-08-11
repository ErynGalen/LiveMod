#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>

void my_func() { printf("my_func\n"); }

int main() {
    my_func();
    const char *sym_name = "_start";
    void *program        = dlopen(NULL, RTLD_NOW);
    void *address        = dlsym(NULL, sym_name);
    Dl_info info;
    if (dladdr(address, &info)) {
        printf("Boo error: %s\n", dlerror());
    }
    printf("Program: %p\nAddress of %s: %p\nError: %s\n", program, sym_name, address, dlerror());
    printf("Info:\n  file: %s, address: %p,\n  nearest symbol: %s, address of "
           "nearest: %p\n",
           info.dli_fname, info.dli_fbase, info.dli_sname, info.dli_saddr);
    return 0;
}
