#ifndef USER_CONTROLLER_H_INCLUDED
#define USER_CONTROLLER_H_INCLUDED

#define MAX_USER_ID 10
#define MAX_USER_NAME 10
#define MAX_USER_IP 20

#include "pipe_controller.h"
typedef struct UserData
{
    int id;
    int fd;
    char *name;
    char *ip;

    Pipe *pipeHead;
} UserData;

typedef struct User
{
    UserData *data;
    struct User *prev;
    struct User *next;
} User;

UserData *user_data_init();
User *user_init();
User *get_user_by_id(User *head, int id);
void log_all_user(User *head, int idx);
void add_user(User *head, User *nuser);
void delete_user(User *user);

int can_change_user_name(User *head, User *user, char *name);
int get_min_id(User *head);

/**
 * @brief Get ip:port string from ip and port
 * @param ip The ip string
 * @param port The port string
 * @return The ip:port string
 **/
char *get_ip_port(char *ip, char *port);
#endif // DOUBLELINK_H_INCLUDED
