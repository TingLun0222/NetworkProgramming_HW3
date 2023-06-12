#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include "pipe_controller.h"
#include "shell_utils.h"
#include "chat_utils.h"
PipeData *pipe_data_init()
{
    PipeData *data = (PipeData *)malloc(sizeof(PipeData));
    data->pipeCnt = -1;
    data->prevOutput = malloc(MAX_OUTPUT_SIZE * sizeof(char *));
}

Pipe *pipe_init()
{
    Pipe *pipe = (Pipe *)malloc(sizeof(Pipe));
    pipe->data = pipe_data_init();
    pipe->next = pipe;
    pipe->prev = pipe;
    return pipe;
}

void add_pipe(Pipe *head, Pipe *npipe)
{
    npipe->next = head;
    npipe->prev = head->prev;
    head->prev->next = npipe;
    head->prev = npipe;
}

void delete_pipe(Pipe *pipe)
{
    pipe->prev->next = pipe->next;
    pipe->next->prev = pipe->prev;
    free(pipe);
}

void decrement_all_pipe(Pipe *head)
{
    Pipe *cur = head->next;
    while (cur != head)
    {
        cur->data->pipeCnt--;
        cur = cur->next;
    }
}

void have_pipe_cmd(Pipe *head, char *output)
{
    Pipe *cur = head->next;
    while (cur != head)
    {
        if (cur->data->pipeCnt == 0)
        {
            strcpy(output, cur->data->prevOutput);
            return;
        }
        cur = cur->next;
    }
    output = NULL;
}

void remove_unuse_pipe(Pipe *head)
{
    Pipe *cur = head->next;
    while (cur != head)
    {
        if (cur->data->pipeCnt == 0)
        {
            delete_pipe(cur);
        }
        cur = cur->next;
    }
}

void log_all_pipe(Pipe *head)
{
    Pipe *cur = head->next;
    while (cur != head)
    {
        PipeData *pdata = cur->data;
        printf("%d: %s\n", pdata->pipeCnt, pdata->prevOutput);
        cur = cur->next;
    }
    printf("----------\n");
}
