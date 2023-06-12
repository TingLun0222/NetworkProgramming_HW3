#ifndef CHAT_UTILS_H_INCLUDED
#define CHAT_UTILS_H_INCLUDED

#define MAX_COMMAND_SIZE 1000
#define MAX_INPUT_SIZE 5000
#define MAX_OUTPUT_SIZE 5000

#include "user_controller.h"
typedef struct ChatArgs
{
    int socketFD;
    User *user;
    User *uhead;
} ChatArgs;

/**
 * @brief: send message to user
 * @param fd: socket file descriptor
 * @param msg: message to send
 * @return: void
 **/
void send_msg(int fd, char *msg);

/**
 * @brief: receive message from user
 * @param fd: socket file descriptor
 * @param msg: message to receive
 * @return: void
 **/
void recv_msg(int fd, char *msg);

/**
 * @brief Convert a string with CRLF to LF
 * @param str The string to be converted
 **/
void crlf_to_lf(char *str);
#endif // CHAT_UTILS_H_INCLUDED