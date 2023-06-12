#ifndef DB_SERVICE_H_INCLUDED
#define DB_SERVICE_H_INCLUDED

#define MAX_USERNAME_LEN 32
#define MAX_PASSWORD_LEN 32

#include <hiredis/hiredis.h>
#include "chat_service.h"

typedef struct DbArgs
{
    int socketFD;
    User *uhead;
    User *user;
} DbArgs;

/**
 * @brief db service entry point
 * @return void
 **/
void *db_client(void *args);

/**
 * @brief connect to redis server
 * @return redis context
 **/
redisContext *connect_redis();

/**
 * @brief user login function
 * @param redis redis context
 * @return username if login success, NULL if failed to execute command
 **/
char *user_login(redisContext *redis, int socketFD);

/**
 * @brief user login function
 * @param redis redis context
 * @param username username
 * @param password password
 * @return 1 if login success, 0 if password incorrect, -1 if account not found
 **/
int login_account(redisContext *redis, const char *username, const char *password);

/**
 * @brief user register function
 * @param redis redis context
 * @param username username
 * @param password password
 * @return 1 if register success, 0 if account already exists, -1 if failed to execute command
 **/
int register_account(redisContext *redis, const char *username, const char *password);

/**
 * @brief mail to specific user
 * @param redis redis context
 * @param sender sender
 * @param receiver receiver
 * @param message message
 * @return void
 **/
void mailto(User *uhead, User *user, int userfd, char *sender, Command *cmd);

/**
 * @brief list mail box of specific user
 * @param redis redis context
 * @param name username
 * @return void
 **/
void listMail(int userfd, char *username);

/**
 * @brief delete mail of specific id
 * @param redis redis context
 * @param name username
 * @param cmd command
 * @return void
 **/
void delMail(int userfd, char *username, Command *cmd);

void createGroup(int userfd, char *username, char *groupName);

void gyell(User *user, int userfd, char *username, char *groupName, Command *cmd);

void delGroup(int userfd, char *username, char *groupName);

void listGroup(int userfd, char *username);

void leaveGroup(int sockfd, char *username, char *groupName);

void addTo(int sockfd, char *username, Command *cmd);
#endif // DB_SERVICE_H_INCLUDED
