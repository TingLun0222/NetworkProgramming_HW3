#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <hiredis/hiredis.h>

#include "chat_service.h"
#include "chat_utils.h"
#include "user_controller.h"
#include "shell_utils.h"
#include "shell_service.h"

void *chat_client(void *args)
{
    ChatArgs *cargs = (ChatArgs *)args;
    int socketFD = cargs->socketFD;
    User *user = cargs->user;
    User *uhead = cargs->uhead;

    start_shell(socketFD);
    // Handle client connection here
    while (1)
    {
        char *sign = "%";
        char *input = malloc(MAX_INPUT_SIZE * sizeof(char *));
        char *output = malloc(MAX_OUTPUT_SIZE * sizeof(char *));

        log_all_user(uhead, user->data->id);
        send_msg(socketFD, sign);

        int num_bytes = recv(socketFD, input, MAX_INPUT_SIZE, 0);
        if (num_bytes == -1)
        {
            perror("recv");
            break;
        }
        else if (num_bytes == 0)
        {
            printf("Client disconnected\n");
            break;
        }

        crlf_to_lf(input);
        printf("Received message from client: %s\n", input);

        int state = shell(uhead, user, input, output, socketFD);
        strcat(output, "\n");
        if (state == -1)
            break;
        else if (state == 0)
            send_msg(socketFD, output);
    }
    end_shell(socketFD);
    delete_user(user);
    close(socketFD);
}

void who(User *uhead, User *user, int socketFD)
{
    int idx = user->data->id;
    char *tmp = malloc(MAX_OUTPUT_SIZE * sizeof(char *));
    char *msg = malloc(MAX_OUTPUT_SIZE * sizeof(char *));

    const char *TID = "<ID>";
    const char *TNAME = "<NAME>";
    const char *TIP = "<IP>";

    snprintf(tmp, MAX_OUTPUT_SIZE, "%4s %10s %20s\n", TID, TNAME, TIP);
    strcpy(msg, tmp);
    User *cur = uhead->next;
    while (cur != uhead)
    {
        UserData *udata = cur->data;
        snprintf(tmp, MAX_OUTPUT_SIZE, "%4d %10s %20s", udata->id, udata->name, udata->ip);
        strcat(msg, tmp);
        if (idx == udata->id)
        {
            snprintf(tmp, MAX_OUTPUT_SIZE, "    <-");
            strcat(msg, tmp);
        }
        snprintf(tmp, MAX_OUTPUT_SIZE, "\n");
        strcat(msg, tmp);
        cur = cur->next;
    }
    send_msg(socketFD, msg);
}

void name(User *uhead, User *user, int socketFD, char *newName)
{
    char *msg = malloc(MAX_OUTPUT_SIZE * sizeof(char *));
    if (can_change_user_name(uhead, user, newName))
    {
        strcpy(user->data->name, newName);
        snprintf(msg, MAX_OUTPUT_SIZE, "Change user name sucess.\n");
    }
    else
    {
        snprintf(msg, MAX_OUTPUT_SIZE, "User name alreaady exist.\n");
    }
    send_msg(socketFD, msg);
}

void tell(User *uhead, User *sender, int socketFD, Command *cmd)
{
    char *msg = malloc(MAX_OUTPUT_SIZE * sizeof(char *));
    int tid = atoi(cmd->args[1]);
    User *receiver = get_user_by_id(uhead, tid);
    snprintf(msg, MAX_OUTPUT_SIZE, "<user(%d) tell you>:", sender->data->id);

    for (int idx = 2; idx < cmd->argc; idx++)
    {
        strcat(msg, cmd->args[idx]);
        strcat(msg, " ");
    }
    strcat(msg, "\n");
    send_msg(receiver->data->fd, msg);

    snprintf(msg, MAX_OUTPUT_SIZE, "send accept.\n");
    send_msg(sender->data->fd, msg);
}

void yell(User *uhead, User *user, int socketFD, Command *cmd)
{
    char *_msg = malloc(MAX_OUTPUT_SIZE * sizeof(char *));
    for (int idx = 1; idx < cmd->argc; idx++)
    {
        strcat(_msg, cmd->args[idx]);
        strcat(_msg, " ");
    }
    strcat(_msg, "\n");

    User *cur = uhead->next;
    while (cur != uhead)
    {
        char *msg = malloc(MAX_OUTPUT_SIZE * sizeof(char *));
        snprintf(msg, MAX_OUTPUT_SIZE, "<user(%d) yell:", user->data->id);
        strcat(msg, _msg);
        send_msg(cur->data->fd, msg);
        cur = cur->next;
    }
}
