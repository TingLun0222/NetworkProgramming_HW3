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
#include "chat_utils.h"
#include "db_service.h"
#include "user_controller.h"

#define PORT 8887

int main()
{
    int server_fd, user_fd;
    struct sockaddr_in my_addr;
    struct sockaddr_in their_addr;
    socklen_t sin_size;
    User *uhead = user_init();

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("socket");
        exit(1);
    }

    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(PORT);
    my_addr.sin_addr.s_addr = INADDR_ANY;
    memset(&(my_addr.sin_zero), '\0', 8);

    if (bind(server_fd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1)
    {
        perror("bind");
        exit(1);
    }

    if (listen(server_fd, 10) == -1)
    {
        perror("listen");
        exit(1);
    }

    printf("Server started and listening on port %d...\n", PORT);

    while (1)
    {
        sin_size = sizeof(struct sockaddr_in);
        if ((user_fd = accept(server_fd, (struct sockaddr *)&their_addr, &sin_size)) == -1)
        {
            perror("accept");
            continue;
        }

        char *_userIP = inet_ntoa(their_addr.sin_addr);
        char *_userPort = int_to_str(ntohs(their_addr.sin_port));
        char *userIP = get_ip_port(_userIP, _userPort);

        printf("New client connected from %s\n", userIP);
        User *user = user_init();
        user->data->id = get_min_id(uhead);
        user->data->fd = user_fd;
        user->data->ip = userIP;

        pthread_t thread;
        DbArgs *dbArgs = (DbArgs *)malloc(sizeof(DbArgs));
        dbArgs->socketFD = user_fd;
        dbArgs->uhead = uhead;
        dbArgs->user = user;
        if (pthread_create(&thread, NULL, db_client, (void *)dbArgs) != 0)
        {
            perror("pthread_create");
            printf("close(user_fd);\n");
	    close(user_fd);
	    printf("main free\n");
            free(dbArgs);
            continue;
        }
	printf("pthread_detach(thread);\n");
        pthread_detach(thread);
    }

    return 0;
}
