#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s program [args...]\n", argv[0]);
        return 1;
    }
    // run lexer
    pid_t pid = fork();

    if (pid == -1) {
        perror("fork failed");
        return 1;
    } else if (pid == 0) {
        // Child process
        execlp("./lexer", "./lexer", argv[1], NULL);
        
        // If execlp returns, it failed
        perror("execlp failed");
        return 1;
    } else {
        // Parent process
        int status;
        waitpid(pid, &status, 0); // Wait for the child process to finish
        if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
            printf("Lexer process exited with non-zero status\n");
            return 1;
        }

        printf("Lexer process completed successfully\n");
    }

    // run parser
    pid = fork();

    if (pid == -1) {
        perror("fork failed");
        return 1;
    } else if (pid == 0) {
        // Child process
        execlp("./q", "./q", NULL);
        
        // If execlp returns, it failed
        perror("execlp failed");
        return 1;
    } else {
        // Parent process
        int status;
        waitpid(pid, &status, 0); // Wait for the child process to finish
        if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
            printf("Parser process exited with non-zero status\n");
            return 1;
        }

        printf("Parser process completed successfully\n");
    }


    // run writer
    pid = fork();

    if (pid == -1) {
        perror("fork failed");
        return 1;
    } else if (pid == 0) {
        // Child process
        execlp("./writer", "./writer", NULL);
        
        // If execlp returns, it failed
        perror("execlp failed");
        return 1;
    } else {
        // Parent process
        int status;
        waitpid(pid, &status, 0); // Wait for the child process to finish
        if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
            printf("Parser process exited with non-zero status\n");
            return 1;
        }

        printf("Writer process completed successfully\n");
    }


    return 0;
}

