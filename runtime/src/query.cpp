#include "query.h"
#include "dlfcn.h"

void *query_symbol(void *handle, const char *name) {
    void *addr = dlsym(handle, name);
    // TODO: handle non-dynamic symbols
    return addr;
}
