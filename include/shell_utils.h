#ifndef SHELL_UTILS_H_INCLUDED
#define SHELL_UTILS_H_INCLUDED

#define READ_END 0
#define WRITE_END 1

#define die(e)                      \
    do                              \
    {                               \
        fprintf(stderr, "%s\n", e); \
        exit(EXIT_FAILURE);         \
    } while (0);
    
typedef struct Command
{
    int argc;
    char **args;
} Command;

Command **get_command_array();

int command_parse(char *command, Command **commands);

char *run_command(Command *command, char *prevOut, int fd);

void print_command(Command *command);

void sigpipe_handler(int signum);
#endif // COMMAND_H_INCLUDED
