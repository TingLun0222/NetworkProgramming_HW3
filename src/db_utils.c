#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <hiredis/hiredis.h>
#include <time.h>

#include "utils.h"
#include "shell_service.h"
#include "chat_service.h"
#include "chat_utils.h"
#include "db_utils.h"
#include "db_service.h"
#include "user_controller.h"
#include "shell_utils.h"

int user_input(char *username, char *password, int socketFD)
{
    send_msg(socketFD, "username: ");
    recv_msg(socketFD, username);
    if (strcmp(username, "exit") == 0)
        return 0;
    send_msg(socketFD, "password: ");
    recv_msg(socketFD, password);
    if (strcmp(password, "exit") == 0)
        return 0;
    return 1;
}

int login_mode(redisContext *redis, char *username, char *password, int socketFD)
{
    while(1)
    {
        send_msg(socketFD, "----- Login start, enter \"exit\" to leave -----\n");
        if (!user_input(username, password, socketFD))
            return 0;
        int loginResult = login_account(redis, username, password);
        if (loginResult == 1)
        {
            send_msg(socketFD, "Login success!\n");
            return 1;
        }
        else if (loginResult == 0)
            send_msg(socketFD, "Password incorrect, please try again.\n");
        else
            send_msg(socketFD, "Account not found!\n");
    }
}
void register_mode(redisContext *redis, char *username, char *password, int socketFD)
{
    while(1)
    {
        if (!user_input(username, password, socketFD))
            return;
        int result = register_account(redis, username, password);
        if (result == 1)
        {
            send_msg(socketFD, "Register success!\n");
            return;
        }
        else if (result == 0)
            send_msg(socketFD, "Account already exists, please try again.\n");
        else
            send_msg(socketFD, "Failed to execute command, please try again.\n");
    }
}

int push_mailto_result(redisContext *redis, User *uhead, User *user, char *sender, char *receiver, char *message, int userfd)
{
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    char current_time[256] = {};
    strftime(current_time, sizeof(current_time), "%Y-%m-%d %H:%M:%S", tm);
    printf("%s\n",current_time);
    // output redirect
    int redirect = 0;
    char *output = (char *)malloc(sizeof(char) * 256);
    if (message[0] == '<')
    {
        redirect = 1;
        removeFirstCharacter(message);
        printf("message: %s\n", message);
        int state = shell(uhead, user, message, output, userfd);
        printf("state: %s\n", output);
    }

    char *identifier = (char *)malloc(sizeof(char) * 256);
    sprintf(identifier, "%s=>MailBox", receiver);

    // get suitable id
    redisReply *reply = execute_redis_command(redis, REDIS_REPLY_INTEGER, "LLEN %s", identifier);
    if (!reply)
        return 0;
    long long id = 1;
    id = reply->integer;
    freeReplyObject(reply);

    char *buffer = (char *)malloc(sizeof(char) * 1024);
    if (redirect == 1)
        strcpy(message, output);
    sprintf(buffer, " %lld\t%s   \t%s \t\t%s\n", id, current_time, sender, message);
    printf("%s\n",message);
    printf("%s\n",buffer);
    reply = execute_redis_command(redis, REDIS_REPLY_INTEGER, "LPUSH %s %s", identifier, buffer);
    if (!reply)
        return 0;
    freeMemory(4, message, output, identifier, buffer);
    return 1;
}
char *get_listGroup_col_string(redisReply *reply, redisReply *reply_nest,int i)
{
    char *group = (char*) malloc(sizeof(char) * 64);
    char *owner = (char*) malloc(sizeof(char) * 64);
    strcpy(group, reply->element[i]->str);
    strcpy(owner, reply_nest->element[0]->str);
    char *msg = (char*) malloc(sizeof(char) * 256);
    sprintf(msg, " %s \t %s  \n", owner, group);
    freeMemory(2, group, owner);
    return msg;
}
void send_addTo_result(int sockfd, char (*arr)[64], int num, char *suffix)
{
    char prompt[1024] = {};
    for (int i = 0; i < num; i++)
    {
        strcat(prompt, arr[i]);
        strcat(prompt, " ");
    }
    if (strcmp(prompt, "") != 0)
    {
        strcat(prompt, suffix);
        send_msg(sockfd, prompt);
    }
}
int set_user_password(redisContext *redis, const char *username, const char *password)
{
    redisReply *reply = execute_redis_command(redis, REDIS_REPLY_STATUS, "SET %s %s", username, password);
    if (!reply)
        return 0;
    freeReplyObject(reply);
    return 1;
}

int get_user_password(redisContext *redis, const char *username, char *password)
{
    redisReply* reply = execute_redis_command(redis, REDIS_REPLY_STRING, "GET %s", username);
    if (!reply)
        return 0;
    strcpy(password, reply->str);
    freeReplyObject(reply);
    return 1;
}

redisReply* execute_redis_command(redisContext *redis, int expected_reply_type, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    redisReply *reply = redisvCommand(redis, format, args);
    va_end(args);
    if (reply == NULL)
        printf("Failed to execute command: %s\n", format);
    else if (reply->type == REDIS_REPLY_ERROR)
    {
        printf("Error executing command: %s\n", reply->str);
        freeReplyObject(reply);
        reply = NULL;
    }
    else if (reply->type != expected_reply_type)
    {
        printf("Unexpected reply type: %d for command: %s\n", reply->type, format);
        freeReplyObject(reply);
        reply = NULL;
    }
    return reply;
}

void freeMemory(int count, ...) {
    va_list args;
    va_start(args, count);
    for (int i = 0; i < count; i++) {
        free(va_arg(args, char*));
    }
    va_end(args);
}

