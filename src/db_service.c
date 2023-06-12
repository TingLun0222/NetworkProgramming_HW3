#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <stdbool.h>
#include <ctype.h>
#include <hiredis/hiredis.h>
#include <time.h>
#include <pthread.h>

#include "utils.h"
#include "shell_service.h"
#include "chat_service.h"
#include "chat_utils.h"
#include "db_utils.h"
#include "db_service.h"
#include "user_controller.h"
#include "shell_utils.h"

void *db_client(void *args)
{
    DbArgs *dbArgs = (DbArgs *)args;
    redisContext *redis = connect_redis();

    // login/register state
    char *username = malloc(sizeof(char) * MAX_USERNAME_LEN);
    username = user_login(redis, dbArgs->socketFD);
    dbArgs->user->data->name = username;
    add_user(dbArgs->uhead, dbArgs->user);

    // start db kernal
    ChatArgs *cargs = (ChatArgs *)malloc(sizeof(ChatArgs));
    cargs->socketFD = dbArgs->socketFD;
    cargs->user = dbArgs->user;
    cargs->uhead = dbArgs->uhead;
    chat_client(cargs);
    //printf("db_ccccccccccccccccccccccccccccccccc\n");
    redisFree(redis);
    //printf("db_lllllllllllllllllllllllllllllllll\n");
}

redisContext *connect_redis()
{
    redisContext *redis = redisConnect("127.0.0.1", 6379);

    if (redis == NULL || redis->err)
    {
        if (redis)
        {
            printf("Error connecting to Redis: %s\n", redis->errstr);
            redisFree(redis);
        }
        else
        {
            printf("Failed to allocate Redis context\n");
        }
        exit(0);
        return NULL;
    }
    printf("Connected to Redis.\n");
    return redis;
}

char *user_login(redisContext *redis, int socketFD)
{
    while (1)
    {
        char *choice = malloc(sizeof(char) * 2);
        char *username = malloc(sizeof(char) * MAX_USERNAME_LEN);
        char *password = malloc(sizeof(char) * MAX_PASSWORD_LEN);
        send_msg(socketFD, "Create account or login again ? <1/2> : ");
        recv_msg(socketFD, choice);
        if (strcmp(choice, "1") == 0)
            register_mode(redis, username, password, socketFD);
        else if(strcmp(choice, "2") == 0)
        {
            if(login_mode(redis, username, password, socketFD));
            {
                freeMemory(2, choice, password);
                return username;
            }
        }
        else if (strcmp(choice, "exit") == 0)
        {
            freeMemory(3, username, password, choice);
            return NULL;
        }
        else
            send_msg(socketFD, "Command not found!\n");
        freeMemory(3, username, password, choice);
    }
}
int login_account(redisContext *redis, const char *username, const char *password)
{
    char user_password[MAX_PASSWORD_LEN] = {};
    int result = get_user_password(redis, username, user_password);
    return (!result) ? -1 : (strcmp(user_password, password) == 0) ? 1 : 0;
}

int register_account(redisContext *redis, const char *username, const char *password)
{
    redisReply *reply = execute_redis_command(redis, REDIS_REPLY_INTEGER, "EXISTS user:%s:password", username);
    if (!reply)
        return -1;
    if (reply->integer == 1)
    {
        freeReplyObject(reply);
        return 0;
    }
    freeReplyObject(reply);
    return set_user_password(redis, username, password) ? 1 : -1;
}

void mailto(User *uhead, User *user, int userfd, char *sender, Command *cmd)
{
    char *receiver = cmd->args[1];
    char *message = malloc(sizeof(char) * 1024);
    for (int idx = 2; idx < cmd->argc; idx++)
    {
        strcat(message, cmd->args[idx]);
        strcat(message, " ");
    }

    redisContext *redis = connect_redis();
    redisReply* reply = execute_redis_command(redis, REDIS_REPLY_STRING, "GET %s", receiver);
    if (!reply) 
	send_msg(userfd, "User not found\n");
    freeReplyObject(reply);

    if (push_mailto_result(redis, uhead, user, sender, receiver, message, userfd))
        send_msg(userfd, (reply && !reply->integer) ? "Send accept!\n" : "Send failed!\n");
}

void listMail(int userfd, char *username)
{
    redisContext *redis = connect_redis();
    redisReply *reply = execute_redis_command(redis, REDIS_REPLY_ARRAY, "LRANGE %s=>MailBox 0 -1", username);
    if(!reply) return;
    if (reply->elements)
    {
        send_msg(userfd, "<id>\t<date>\t\t\t<sender>\t<message>\n");
        for (size_t i = 0; i < reply->elements; i++)
            send_msg(userfd, reply->element[i]->str);
    }
    else if (!reply->elements)
        send_msg(userfd, "empty !\n");
    freeReplyObject(reply);
}

void delMail(int userfd, char *username, Command *cmd)
{
    redisContext *redis = connect_redis();
    char *id = cmd->args[1];
    redisReply *reply = execute_redis_command(redis, REDIS_REPLY_ARRAY, "LRANGE %s=>MailBox 0 -1", username);
    if(!reply) return;
    char *delMsg = (char *)malloc(sizeof(char) * 1024);
    char *check = (char *)malloc(sizeof(char) * 2);
    for (size_t i = 0; i < reply->elements; i++)
    {
        sscanf(reply->element[i]->str, " %s", check);
        if (strcmp(check, id) == 0)
        {
            strcpy(delMsg, reply->element[i]->str);
            break;
        }
    }
    freeReplyObject(reply);
    reply = execute_redis_command(redis, REDIS_REPLY_INTEGER, "LREM %s=>MailBox 0 %s", username, delMsg);
    if(!reply) return;
    send_msg(userfd, (reply->integer) ? "Delete success!\n" : "Delete failed!\n");
    freeMemory(2, delMsg, check);
    freeReplyObject(reply);
}

void createGroup(int userfd, char *username, char *groupName)
{
    redisContext *redis = connect_redis();
    redisReply *reply = execute_redis_command(redis, REDIS_REPLY_INTEGER, "ZCARD groupList");
    if(!reply) return;
    int gid = reply->integer;
    freeReplyObject(reply);

    reply = execute_redis_command(redis, REDIS_REPLY_INTEGER, "ZADD groupList %d %s", gid + 1, groupName);
    if(!reply) return;
    if (reply->integer)
    {
        redisReply *tr = execute_redis_command(redis, REDIS_REPLY_INTEGER, "ZADD %s 1 %s", groupName, username);
        if(!tr) return;
        freeReplyObject(tr);
        send_msg(userfd, "Create success !\n");
    }
    else
    {
        send_msg(userfd, "Group already exist !\n");
    }
    freeReplyObject(reply);
}

void delGroup(int userfd, char *username, char *groupName)
{
    redisContext *redis = connect_redis();
    redisReply *reply = execute_redis_command(redis, REDIS_REPLY_ARRAY, "ZRANGE %s 0 -1", groupName);
    if(!reply) return;
    if (reply->elements == 0)
    {
        send_msg(userfd, "Group not found !\n");
        freeReplyObject(reply);
        return;
    }

    if (strcmp(reply->element[0]->str, username) == 0)
    {
        redisReply *tr = execute_redis_command(redis, REDIS_REPLY_INTEGER, "DEL %s", groupName);
        if(!tr) return;
        send_msg(userfd, "Group delete success !\n");
        freeReplyObject(tr);

        tr = execute_redis_command(redis, REDIS_REPLY_INTEGER, "ZREM groupList %s", groupName);
        if(!tr) return;
        freeReplyObject(tr);
    }
    else
        send_msg(userfd, "You don't have permissions !\n");
    freeReplyObject(reply);
}
void gyell(User *uhead, int userfd, char *username, char *groupName, Command *cmd)
{
    redisContext *redis = connect_redis();
    redisReply *reply = execute_redis_command(redis, REDIS_REPLY_ARRAY, "ZSCAN groupList 0 match %s", groupName);
    //redisReply *reply = redisCommand(redis, "ZSCAN groupList 0 match %s", groupName);
    if(!reply) return;
    if (!reply->element[1]->element)
    {
        send_msg(userfd, "Group not found !\n");
        freeReplyObject(reply);
        return;
    }

    char *message = malloc(sizeof(char) * 1024);
    for (int idx = 2; idx < cmd->argc; idx++)
    {
        strcat(message, cmd->args[idx]);
        strcat(message, " ");
    }

    reply = execute_redis_command(redis, REDIS_REPLY_ARRAY, "ZRANGE %s 0 -1", groupName);
    if(!reply) return;
    for (int i = 0; i < reply->elements; i++)
        for (User *cur = uhead->next; cur != uhead; cur = cur->next)
            if (strcmp(cur->data->name, reply->element[i]->str) == 0)
            {
                char *to_send = malloc(sizeof(char) * 2048);
                sprintf(to_send, "<%s-%s>: %s\n", groupName, username, message);
                send_msg(cur->data->fd, to_send);
                free(to_send);
            }
    free(message);
}
void listGroup(int sockfd, char *username)
{
    redisContext *redis = connect_redis();
    redisReply *reply = execute_redis_command(redis, REDIS_REPLY_ARRAY, "ZRANGE groupList 0 -1");
    if (!reply) return ;
    int flag = 0;
    int first = 1;
    for (size_t i = 0; i < reply->elements; i++)
    {
        redisReply *reply_nest = execute_redis_command(redis, REDIS_REPLY_ARRAY, "ZRANGE %s 0 -1", reply->element[i]->str);
        if (!reply || !reply_nest->elements) return ;
        char *msg = get_listGroup_col_string(reply, reply_nest, i);
        for (int j = 0; j < reply_nest->elements; j++)
        {
            if (strcmp(reply_nest->element[j]->str, username) == 0)
            {
                if (first)
                {
                    send_msg(sockfd, " <owner> \t <group>  \n");
                    first = 0;
                }
                flag = 1;
                send_msg(sockfd, msg);
                break;
            }
        }
        free(msg);
        freeReplyObject(reply_nest);
    }
    if (flag == 0)
        send_msg(sockfd, "Empty !\n");
    freeReplyObject(reply);
}

void leaveGroup(int sockfd, char *username, char *groupName)
{
    redisContext *redis = connect_redis();
    redisReply *reply = execute_redis_command(redis, REDIS_REPLY_ARRAY, "ZSCAN groupList 0 match %s", groupName);
    if (!reply) return ;
    if (!reply->element[1]->element)
    {
        send_msg(sockfd, "Group not found !\n");
        freeReplyObject(reply);
        return;
    }
    freeReplyObject(reply);
    reply = execute_redis_command(redis, REDIS_REPLY_INTEGER, "ZREM %s %s", groupName, username);
    if (!reply) return;
    if (reply->integer == 0)
    {
        send_msg(sockfd, "Leave fault !\n");
        freeReplyObject(reply);
        return;
    }
    redisReply *tr = execute_redis_command(redis, REDIS_REPLY_ARRAY, "ZRANGE %s 0 -1", groupName);
    if (!tr) return ;
    if (tr->elements == 1)
    {
        freeReplyObject(tr);
        tr = execute_redis_command(redis, REDIS_REPLY_INTEGER, "ZREM groupList %s", groupName);
        if (!tr) return;
        freeReplyObject(tr);
    }
    send_msg(sockfd, "Leave accept !\n");
    freeReplyObject(reply);
}

void addTo(int sockfd, char *username, Command *cmd)
{
    char *groupName = cmd->args[1];
    char success[100][64] = {};
    char notFound[100][64] = {};
    char inGroup[100][64] = {};
    int pos = 0, count = 0, nextPos = 0;
    int success_num = 0, notFound_num = 0, inGroup_num = 0;
    redisReply *tr;

    redisContext *redis = connect_redis();
    redisReply *reply = execute_redis_command(redis, REDIS_REPLY_ARRAY, "ZRANGE %s 0 -1", groupName);
    if (!reply) return;
    if (reply->elements == 0)
    {
        send_msg(sockfd, "Group Not Found !\n");
        freeReplyObject(reply);
        return;
    }
    if (strcmp(reply->element[0]->str, username) != 0)
    {
        send_msg(sockfd, "You don't have permissions !\n");
        freeReplyObject(reply);
        return;
    }
    // user not found
    for (int i = 2; i < cmd->argc; i++)
    {
        tr = execute_redis_command(redis, REDIS_REPLY_STRING, "GET %s", cmd->args[i]);
        if (!tr)
        {
            strcpy(notFound[notFound_num++], cmd->args[i]);
            strcpy(cmd->args[i], "");
        }
        else
            freeReplyObject(tr);
    }

    // user already in group
    for (int i = 0; i < reply->elements; i++)
        for (int j = 2; j < cmd->argc; j++)
            if (strcmp(reply->element[i]->str, cmd->args[j]) == 0)
            {
                strcpy(inGroup[inGroup_num++], cmd->args[j]);
                strcpy(cmd->args[j], "");
            }

    // add user into group
    int gid = reply->elements + 1;
    for (int i = 2; i < cmd->argc; i++)
        if (strcmp(cmd->args[i], "") != 0)
        {
            tr = execute_redis_command(redis, REDIS_REPLY_INTEGER, "ZADD %s %d %s", groupName, gid++, cmd->args[i]);
            if (!tr) return ;
            freeReplyObject(tr);
            strcpy(success[success_num++], cmd->args[i]);
        }

    send_addTo_result(sockfd, inGroup, inGroup_num, "already in group !\n");
    send_addTo_result(sockfd, notFound, notFound_num, "not found !\n");
    send_addTo_result(sockfd, success, success_num, "add success !\n");

    freeReplyObject(reply);
}

