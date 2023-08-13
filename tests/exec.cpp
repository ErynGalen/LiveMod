#include <cerrno>
#include <cstring>
#include <iostream>
#include <string>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

int main() {
    std::string command           = "echo";
    std::vector<std::string> args = {"boo", "2nd"};

    int pipe_fds[2];
    if (pipe(pipe_fds) == -1) {
        printf("Couldn't exec\n");
    }
    int pid = fork();
    if (pid == 0) { // child process
        std::cout << "Child" << std::endl;
        close(pipe_fds[0]); // close read-end of the pipe
        std::cout << "Exec <" << command;
        {
            for (std::string &arg : args) {
                std::cout << " " << arg;
            }
            std::cout << ">" << std::endl;
        }
        dup2(pipe_fds[1], STDOUT_FILENO);

        // build arg array
        char **args_array = (char **)malloc(sizeof(char *) * (args.size() + 2));
        args_array[0]     = (char *)malloc(sizeof(char) * (command.size() + 1));
        command.copy(args_array[0], command.size());
        args_array[0][command.size()] = 0;

        size_t n = 1;
        for (std::string &arg : args) {
            args_array[n] = (char *)malloc(sizeof(char) * (arg.size() + 1));
            arg.copy(args_array[n], arg.size());
            args_array[n][arg.size()] = 0;
            n++;
        }
        args_array[n] = nullptr;

        int ret = execvp(command.c_str(), args_array);
        exit(42);
    }
    // parent process
    std::cout << "Parent" << std::endl;
    close(pipe_fds[1]); // close write-end of the pipe
    int child_status;
    std::cout << "waiting for child" << std::endl;
    waitpid(pid, &child_status, 0);
    std::cout << "Child terminated (" << child_status << ")" << std::endl;

    char buffer[128];
    buffer[sizeof(buffer) - 1] = 0;
    size_t sizeRead            = 0;
    while ((sizeRead = read(pipe_fds[0], buffer, sizeof(buffer) - 1)) > 0) {
        buffer[sizeRead] = 0;
        std::cout << buffer << std::endl;
    }
    return 0;
}
