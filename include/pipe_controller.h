#ifndef PIPE_CONTROLLER_H_INCLUDED
#define PIPE_CONTROLLER_H_INCLUDED

#define MAX_USER_ID 10
#define MAX_USER_NAME 10
#define MAX_USER_IP 20

typedef struct PipeData
{
    int pipeCnt;
    char *prevOutput;
} PipeData;

typedef struct Pipe
{
    PipeData *data;
    struct Pipe *prev;
    struct Pipe *next;
} Pipe;

PipeData *pipe_data_init();
Pipe *pipe_init();

void add_pipe(Pipe *head, Pipe *npipe);
void delete_pipe(Pipe *pipe);
void decrement_all_pipe(Pipe *head);
void have_pipe_cmd(Pipe *head, char *output);
void remove_unuse_pipe(Pipe *head);
void log_all_pipe(Pipe *head);
#endif // DOUBLELINK_H_INCLUDED
