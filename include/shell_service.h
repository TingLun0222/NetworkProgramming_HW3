#ifndef SHELL_SERVICE_H_INCLUDED
#define SHELL_SERVICE_H_INCLUDED

#include "user_controller.h"
#include "shell_utils.h"

/**
 * @brief execute shell command
 * 
 * @param user user
 * @param input input
 * @param output output
 * @return status of the command, -1 means exit, 0 means normal, 1 means pipe 
 */
int shell(User *uhead, User *user, char *input, char *output, int fd);

void run_setenv(Command *command);

void start_shell(int fd);

void end_shell(int fd);
#endif // SHELL_SERVICE_H_INCLUDED
