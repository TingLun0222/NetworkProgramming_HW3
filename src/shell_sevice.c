#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/wait.h>

#include "chat_utils.h"
#include "shell_service.h"
#include "chat_service.h"
#include "shell_utils.h"
#include "user_controller.h"
#include "pipe_controller.h"
#include "db_service.h"

int shell(User *uhead, User *user, char *input, char *output, int fd)
{
    int isPipe = 0;
    UserData *udata = user->data;
    Command **commands = get_command_array();
    int cmdc = command_parse(input, commands);

    if (cmdc == 0)
        return 0;
    else if (strcmp(commands[0]->args[0], "exit") == 0)
        return -1;

    have_pipe_cmd(udata->pipeHead, output);
    for (int idx = 0; idx < cmdc; idx++)
    {
        Command *cmd = commands[idx];

        if (strcmp(cmd->args[0], "setenv") == 0)
        {
            run_setenv(cmd);
            break;
        }
        else if (strcmp(cmd->args[0], "who") == 0)
        {
            who(uhead, user, fd);
            break;
        }
        else if (strcmp(cmd->args[0], "name") == 0)
        {
            name(uhead, user, fd, cmd->args[1]);
            break;
        }
        else if (strcmp(cmd->args[0], "tell") == 0)
        {
            tell(uhead, user, fd, cmd);
            break;
        }
        else if (strcmp(cmd->args[0], "yell") == 0)
        {
            yell(uhead, user, fd, cmd);
            break;
        }
        else if (strcmp(cmd->args[0], "mailto") == 0)
        {
            mailto(uhead, user, user->data->fd, user->data->name, cmd);
            break;
        }
        else if (strcmp(cmd->args[0], "listMail") == 0)
        {
            listMail(fd, user->data->name);
            break;
        }
        else if (strcmp(cmd->args[0], "delMail") == 0)
        {
            delMail(fd, user->data->name, cmd);
            break;
        }
        else if (strcmp(cmd->args[0], "gyell") == 0)
        {
            gyell(uhead, fd, user->data->name, cmd->args[1], cmd);
            break;
        }
        else if (strcmp(cmd->args[0], "createGroup") == 0)
        {
            createGroup(fd, user->data->name, cmd->args[1]);
            break;
        }
        else if (strcmp(cmd->args[0], "delGroup") == 0)
        {
            delGroup(fd, user->data->name, cmd->args[1]);
            break;
        }
        else if (strcmp(cmd->args[0], "listGroup") == 0)
        {
            listGroup(fd, user->data->name);
            break;
        }
        else if (strcmp(cmd->args[0], "leaveGroup") == 0)
        {
            leaveGroup(fd, user->data->name, cmd->args[1]);
            break;
        }
        else if (strcmp(cmd->args[0], "addTo") == 0)
        {
            addTo(fd, user->data->name, cmd);
            break;
        }
        else if (cmd->args[0][0] == '|')
        {
            int cnt = atoi(cmd->args[0] + 1);
            Pipe *npipe = pipe_init();
            npipe->data->pipeCnt = cnt;
            npipe->data->prevOutput = output;
            add_pipe(udata->pipeHead, npipe);
            isPipe = 1;
            break;
        }

        output = run_command(cmd, output, fd);
    }
    if (strlen(output) > 0)
    {
        decrement_all_pipe(udata->pipeHead);
    }
    return isPipe;
}

void run_setenv(Command *command)
{
    if (command->argc != 3)
    {
        printf("setenv error: invalid argument.\n");
        return;
    }
    setenv(command->args[1], command->args[2], 1);
}

void start_shell(int fd)
{
    char *LOGINM = "----- Shell start, enter \"exit\" to leave -----\n";
    send_msg(fd, LOGINM);
    setenv("PATH", "/bin:./bin", 1);
}

void end_shell(int fd)
{
    char *LOGOUTM = "----- Shell start, enter \"exit\" to leave -----\n";
    send_msg(fd, LOGOUTM);
}