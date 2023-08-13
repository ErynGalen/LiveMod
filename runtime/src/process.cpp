#include "process.h"

#include "GlobalContext.h"
#include <cstdio>
#include <cstring>

#ifdef __linux__
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#else
#error Only Linux is currently supported
#endif

std::string execAndGet(char *command, char *const args[]) {
    std::string result;
    if (args == NULL || strcmp(command, args[0]) != 0) {
        printf("[execAndGet] The first argument must be the program name\n");
        return result;
    }

    int pipe_fds[2];
    if (pipe(pipe_fds) == -1) {
        printf("[execAndGet] Couldn't exec\n");
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
    size_t sizeRead = 0;
    while ((sizeRead = read(pipe_fds[0], buffer, sizeof(buffer) - 1)) > 0) {
        buffer[sizeRead] = 0; // terminate C string
        result.append(buffer);
    }

    return result;
}
