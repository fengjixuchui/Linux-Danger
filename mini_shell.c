#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

// Maximum command length
#define MAX_CMD_LEN 256
#define MAX_ARGS 16

char cwd[MAX_CMD_LEN];

void execute_command(char *cmd) {
    char *args[MAX_ARGS];
    char *token = strtok(cmd, " ");

    int i = 0;
    while (token != NULL && i < MAX_ARGS - 1) {
        args[i] = token;
        token = strtok(NULL, " ");
        i++;
    }
    args[i] = NULL;

    // Empty command
    if (i == 0) {
        return;
    }

    // Built-in command: exit
    if (strcmp(args[0], "exit") == 0) {
        printf("Exiting mini shell...\n");
        exit(0);
    }

    // Built-in command: readsegs, will read CS, DS, SS
    if (strcmp(args[0], "readsegs") == 0) {
        unsigned int cs, ds, ss;
        asm volatile("mov %%cs, %0" : "=r"(cs));
        asm volatile("mov %%ds, %0" : "=r"(ds));
        asm volatile("mov %%ss, %0" : "=r"(ss));
        printf("CS = 0x%x, DS = 0x%x, SS = 0x%x\n", cs, ds, ss);
        return;
    }

    // Built-in command: cd
    if (strcmp(args[0], "cd") == 0) {
        if (i < 2) {
            printf("Usage: cd <directory>\n");
        } else {
            if (chdir(args[1]) < 0) {
                perror("cd failed");
            }
            else {
                getcwd(cwd, sizeof(cwd));
            }
        }
        return;
    }

    // Fork a child process
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");
        return;
    }

    if (pid == 0) {
        // Child process
        execvp(args[0], args);
        perror("exec failed");
        exit(1);
    } else {
        // Parent process waits
        int status;
        waitpid(pid, &status, 0);
    }
}

int main() {
    char cmd[MAX_CMD_LEN];
    getcwd(cwd, sizeof(cwd));
    printf("Welcome to mini shell (Ring0 Usermode Test)\n");
    while (1) {
        printf("mini_shell:%s>", cwd);
        if (fgets(cmd, MAX_CMD_LEN, stdin) == NULL) {
            printf("\nExiting mini shell...\n");
            break;
        }
        // Remove newline
        cmd[strcspn(cmd, "\n")] = '\0';
        execute_command(cmd);
    }
    return 0;
}