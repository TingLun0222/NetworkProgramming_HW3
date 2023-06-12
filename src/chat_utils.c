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

#include "user_controller.h"
#include "chat_utils.h"
#include "utils.h"

void send_msg(int fd, char *msg)
{
    if (send(fd, msg, strlen(msg), 0) == -1)
    {
        perror("send");
        close(fd);
        exit(1);
    }
}

void recv_msg(int fd, char *msg)
{
    int state = recv(fd, msg, MAX_INPUT_SIZE, 0);
    if (state == -1)
    {
        perror("recv");
        close(fd);
        exit(1);
    }
    crlf_to_lf(msg);
}

void crlf_to_lf(char *str)
{
    char *p = str;
    char *q = str;
    while (*p)
    {
        if (*p == '\r' && *(p + 1) == '\n')
        {
            p++;
        }
        *q++ = *p++;
    }
    *q = '\0';
    q--;
    *q = '\0';
}