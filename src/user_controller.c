#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include "utils.h"
#include "user_controller.h"
#include "pipe_controller.h"

UserData *user_data_init()
{
    UserData *data = (UserData *)malloc(sizeof(UserData));
    data->id = 0;
    data->fd = -1;
    data->name = malloc(MAX_USER_NAME * sizeof(char *));
    strcpy(data->name, "no name");
    data->ip = malloc(MAX_USER_IP * sizeof(char *));
    data->pipeHead = pipe_init();
    return data;
}

User *user_init()
{
    User *user = (User *)malloc(sizeof(User));
    user->data = user_data_init();
    user->next = user;
    user->prev = user;
    return user;
}

User *get_user_by_id(User *head, int id)
{
    User *cur = head->next;
    while (cur != head)
    {
        UserData *udata = cur->data;
        if(udata->id == id)
            return cur;
        cur = cur->next;
    }
    printf("target not exist\n");
    return head;
}
void log_all_user(User *head, int idx)
{
    const char *TID = "<ID>";
    const char *TNAME = "<NAME>";
    const char *TIP = "<IP>";

    User *cur = head->next;
    printf("%4s %10s %20s\n", TID, TNAME, TIP);
    while (cur != head)
    {
        UserData *udata = cur->data;
        printf("%4d %10s %20s", udata->id, udata->name, udata->ip);
        if (idx == udata->id)
            printf("<-");
        printf("\n");
        cur = cur->next;
    }
}

void add_user(User *head, User *nuser)
{
    nuser->next = head;
    nuser->prev = head->prev;
    head->prev->next = nuser;
    head->prev = nuser;
}
void delete_user(User *user)
{
    user->prev->next = user->next;
    user->next->prev = user->prev;
    if(!user)
    	printf("delete_user failed!\n\n");
    free(user);
}

int can_change_user_name(User *head, User *user, char *name)
{
    User *cur = head->next;
    while (cur != head)
    {
        if (strcmp(cur->data->name, name) == 0)
        {
            printf("User already exist.\n");
            return 0;
        }
        cur = cur->next;
    }
    printf("Name change success.\n");
    strcpy(user->data->name, name);
    return 1;
}

int get_min_id(User *head)
{
    int record[MAX_USER_ID];
    memset(record, 0, MAX_USER_ID);
    int cnt = 0;
    User *cur = head->next;
    while (cur != head)
    {
        record[cur->data->id] = 1;
        cnt++;
        cur = cur->next;
    }
    for (int idx = 0; idx < MAX_USER_ID; idx++)
    {
        if (!record[idx])
            return idx;
    }
    return cnt; // out of bound
}

char *get_ip_port(char *ip, char *port)
{
    strcat(ip, ":");
    strcat(ip, port);
    return ip;
}
