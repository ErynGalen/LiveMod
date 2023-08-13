#include "dlhook.h"
#include "../../GlobalContext.h"
#include "../../query.h"

#include <cstring>
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/cdefs.h>

bool linkOriginal(void **orig, const char *name) {
    if (*orig) { // orig is already linked
        return false;
    }
    *orig = querySymbol(QUERY_NEXT, name);

    return true;
}

extern char **environ;

// hook `fork()` so that child processes aren't LD_PRELOADed
DL_HOOK pid_t fork() {
    DL_ORIG(pid_t, fork);

    pid_t pid = orig_fork();

    if (pid == 0) {
        // in child process, fix the environment
        for (int n = 0; environ[n] != NULL; n++) {
            static char LD_PRELOAD[] = "LD_PRELOAD";
            if (strncmp(environ[n], LD_PRELOAD, sizeof(LD_PRELOAD) - 1) == 0) {
                environ[n][0] = 'D'; // don't LD_PRELOAD the child process
            }
        }

        return pid;
    }
    return pid; // parent process
}
