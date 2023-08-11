#include "process.h"

#include "GlobalContext.h"
#include "dlhook.h"
#include <cstring>

#ifdef __linux__
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#else
#error Only Linux is currently supported
#endif

// hook `fork()` so that child processes aren't LD_PRELOADed
DYNHOOK pid_t fork() {
    static pid_t (*orig)() = NULL;
    link_original((void **)&orig, "fork");

    pid_t pid = orig();
    if (g_Context.m_isNative) {
        return pid;
    }

    if (pid == 0) {
        // in child process, fix the environment
        for (int n = 0; environ[n] != NULL; n++) {
            static char LD_PRELOAD[] = "LD_PRELOAD";
            if (strncmp(environ[n], LD_PRELOAD, sizeof(LD_PRELOAD)) == 0) {
                environ[n][0] = 'D'; // don't LD_PRELOAD the child process
            }
        }

        return pid;
    }
    return pid; // parent process
}

std::string execAndGet(char *command, char *const args[]) {
    std::string result;
    if (args == NULL || strcmp(command, args[0]) != 0) {
        printf("The first argument must be the program name\n");
        return result;
    }

    int pipe_fds[2];
    if (pipe(pipe_fds) == -1) {
        printf("Couldn't exec\n");
    }
    int pid = fork();
    if (pid == 0) {         // child process
        close(pipe_fds[0]); // close read-end of the pipe
        dup2(pipe_fds[1], STDOUT_FILENO);

        execvp(command, args);
    }
    // parent process
    close(pipe_fds[1]); // close write-end of the pipe

    int child_status;
    waitpid(pid, &child_status, 0);

    char buffer[128];
    buffer[sizeof(buffer) - 1] = 0;
    while (read(pipe_fds[0], buffer, sizeof(buffer) - 1) > 0) {
        result.append(buffer);
    }

    return result;
}
