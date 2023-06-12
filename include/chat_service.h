#ifndef CHAT_SERVICE_H_INCLUDED
#define CHAT_SERVICE_H_INCLUDED

#include "user_controller.h"
#include "shell_utils.h"

#define MAX_MAIL_LEN 1024

/**
 * @brief: chat service
 * @param arg: chat arguments
 * @return: void
 **/
void *chat_client(void *arg);


/**
 * @brief display the current user
 * @param uhead: user list head
 * @param user: current user
 * @param socketFD: socket file descriptor
 * @return void
 **/
void who(User *uhead, User *user, int socketFD);

/**
 * @brief change the current user's name
 * @param uhead: user list head
 * @param user: current user
 * @param socketFD: socket file descriptor
 * @param newName: new name
 * @return void
 **/
void name(User *uhead, User *user, int socketFD, char *newName);

/**
 * @brief tell message to user
 * @param uhead: user list head
 * @param user: current user
 * @param socketFD: socket file descriptor
 * @param cmd: command
 * @return void
 **/
void tell(User *uhead, User *user, int socketFD, Command *cmd);

/**
 * @brief yell message to all users
 * @param uhead: user list head
 * @param user: current user
 * @param socketFD: socket file descriptor
 * @param cmd: command
 * @return void
 **/
void yell(User *uhead, User *user, int socketFD, Command *cmd);

#endif // CHAT_SERVICE_H_INCLUDED
