#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

int main() {
    char *command = "echo";
    char *args[]  = {"echo", "boo", "2nd", NULL};

    int pipe_fds[2];
    if (pipe(pipe_fds) == -1) {
        printf("Couldn't exec\n");
    }
    int pid = fork();
    if (pid == 0) { // child process
        printf("Child\n");
        close(pipe_fds[0]); // close read-end of the pipe
        printf("Exec <%s", command);
        {
            int n = 0;
            while (args[n]) {
                printf(" %s", args[n]);
                n++;
            }
            printf(">\n");
        }
        dup2(pipe_fds[1], STDOUT_FILENO);

        execvp(command, args);
        exit(42);
    }
    // parent process
    printf("Parent\n");
    close(pipe_fds[1]); // close write-end of the pipe
    int child_status;
    printf("waiting for child\n");
    waitpid(pid, &child_status, 0);
    printf("Child terminated (%d)\n", child_status);

    char buffer[128];
    buffer[sizeof(buffer) - 1] = 0;
    while (read(pipe_fds[0], buffer, sizeof(buffer) - 1) > 0) {
        printf("%s\n", buffer);
    }
    return 0;
}
