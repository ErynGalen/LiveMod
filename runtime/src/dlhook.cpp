#include "dlhook.h"
#include "GlobalContext.h"
#include "query.h"

#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/cdefs.h>

bool link_original(void **orig, const char *name) {
    if (*orig) { // orig is already linked
        return false;
    }
    *orig = query_symbol(QUERY_NEXT, name);

    return true;
}

DYNHOOK int puts(const char *str) {
    static int (*orig)(const char *str) = NULL;
    link_original((void **)&orig, "puts");
    if (g_Context.m_isNative) {
        return orig(str);
    }

    MakeNativeGuard gn;
    int ret = printf("[puts] %s\n", str);

    return ret;
}
