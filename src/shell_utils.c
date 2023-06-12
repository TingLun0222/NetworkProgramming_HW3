#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/wait.h>

#include "chat_utils.h"
#include "shell_utils.h"

Command **get_command_array()
{
    Command **commands = malloc(MAX_COMMAND_SIZE * sizeof(Command *));
    for (int i = 0; i < MAX_COMMAND_SIZE; i++)
    {
        commands[i] = malloc(sizeof(Command));
        commands[i]->argc = 0;
        commands[i]->args = malloc(MAX_COMMAND_SIZE * sizeof(char *));
        for (int j = 0; j < MAX_COMMAND_SIZE; j++)
        {
            commands[i]->args[j] = NULL;
        }
    }
    commands[MAX_COMMAND_SIZE - 1] = NULL;

    return commands;
}

int command_parse(char *command, Command **commands)
{
    int cmdc = 0;
    char *delim = " \n";
    char *arg = strtok(command, delim);
    if (arg == NULL)
        return 0;

    while (arg != NULL)
    {
        if (arg[0] == '|')
        {
            cmdc++;
            if (strlen(arg) > 1)
            {
                int argc = commands[cmdc]->argc;
                commands[cmdc]->args[argc] = arg;
                commands[cmdc]->argc++;
            }
        }
        else
        {
            int argc = commands[cmdc]->argc;
            commands[cmdc]->args[argc] = arg;
            commands[cmdc]->argc++;
        }
        arg = strtok(NULL, delim);
    }
    return cmdc + 1;
}

char *run_command(Command *command, char *prevOut, int fd)
{
    signal(SIGPIPE, sigpipe_handler);
    int save_in;
    save_in = dup(STDIN_FILENO);

    // copy prevOut to currentCommandOutput and clear prevOut
    char currentCommandOutput[MAX_COMMAND_SIZE];
    memset(currentCommandOutput, 0, MAX_COMMAND_SIZE);
    strcat(currentCommandOutput, prevOut);
    memset(prevOut, 0, MAX_COMMAND_SIZE);

    int link1[2];
    int link2[2];
    pid_t pid1, pid2;
    char buffer[4096 + 1];

    if (pipe(link1) == -1)
        die("pipe1");
    if (pipe(link2) == -1)
        die("pipe2");

    if ((pid1 = fork()) == -1)
        die("fork1");

    if (pid1 == 0)
    {
        // child 1
        dup2(link1[READ_END], STDIN_FILENO);
        dup2(link2[WRITE_END], STDOUT_FILENO);
        close(link1[WRITE_END]);
        close(link2[READ_END]);
        execvp(command->args[0], command->args);

        char *error = malloc(128 * sizeof(char));
        strcpy(error, "Unknown command: [");
        strcat(error, command->args[0]);
        strcat(error, "]");
        send_msg(fd, error);
        die(error);
    }
    else
    {
        if ((pid2 = fork()) == -1)
            die("fork2");
        if (pid2 == 0)
        {
            // child 2
            dup2(link1[WRITE_END], STDOUT_FILENO);
            close(link1[READ_END]);
            close(link2[READ_END]);
            close(link2[WRITE_END]);
            execl("./bin/echo", "-e", currentCommandOutput, (char *)0);
            die("echo: command not found");
        }
        else
        {
            // parent
            dup2(link2[READ_END], STDIN_FILENO);
            close(link1[READ_END]);
            close(link1[WRITE_END]);
            close(link2[WRITE_END]);
            int nbytes = 0;
            while (0 != (nbytes = read(link2[READ_END], buffer, sizeof(buffer))))
            {
                memset(prevOut, 0, MAX_COMMAND_SIZE);
                strcat(prevOut, buffer);
                memset(buffer, 0, 4096);
            }
            prevOut[strlen(prevOut) - 1] = '\0';
            wait(NULL);
        }
        wait(NULL);
    }

    // return to the original stdin for parent process
    dup2(save_in, STDIN_FILENO);
    return prevOut;
}

void print_command(Command *command)
{
    for (int c = 0; c < command->argc; c++)
        printf("%s ", command->args[c]);
    printf("\n");
}

void sigpipe_handler(int signum)
{
}