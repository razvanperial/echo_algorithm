#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "echo.h"

// function to add a message to the buffer
void addMessage(int src, int dest, char *data, message_t **buffer, pthread_mutex_t *buffer_mutex, int *flags, pthread_mutex_t *flags_mutex)
{
    message_t *newMessage = (message_t *)malloc(sizeof(message_t));
    newMessage->data = (char *)malloc(strlen(data) + 1); // Allocate memory for data
    strcpy(newMessage->data, data);                      // Copy data to new memory
    newMessage->next = NULL;
    newMessage->src = src;

    pthread_mutex_lock(flags_mutex);
    if (flags[dest] == 0)
    {
        flags[dest] = 1;
        buffer[dest] = newMessage;
        newMessage->next = NULL;
        pthread_mutex_unlock(&buffer_mutex[dest]);
    }
    else
    {
        pthread_mutex_lock(&buffer_mutex[dest]);

        newMessage->next = buffer[dest];
        buffer[dest] = newMessage;
        pthread_mutex_unlock(&buffer_mutex[dest]);
    }
    pthread_mutex_unlock(flags_mutex);
}

// function to receive a message from the buffer
received_message_t *receiveMessage(int dest, message_t **buffer)
{
    if (buffer[dest] == NULL)
    {
        return NULL;
    }
    else
    {
        message_t *temp = buffer[dest];
        buffer[dest] = buffer[dest]->next;
        received_message_t *received = (received_message_t *)malloc(sizeof(received_message_t));
        received->data = temp->data;
        received->src = temp->src;
        free(temp);
        return received;
    }
}

// initiator function
void *initiator(void *arg)
{
    threadargs_t *args = (threadargs_t *)arg;
    process_t *process = args->process;
    pthread_mutex_t *buffer_mutex = args->buffer_mutex;
    int *flags = args->flags;
    pthread_mutex_t *flags_mutex = args->flags_mutex;
    message_t **buffer = args->buffer;

    // send messages to all neighbors
    Node *temp = process->neigh;
    Node *iter = process->neigh;
    while (iter)
    {
        printf("Initiator %d:\t  Sending message to %d\n", process->id, iter->dest);
        addMessage(process->id, iter->dest, "p1", buffer, buffer_mutex, flags, flags_mutex);
        iter = iter->next;
    }
    process->neigh = temp;

    // receive messages from all neighbors
    while (process->rec < process->neigh_size)
    {
        // printf("Initiator %d:\t  Waiting for message\n", process->id);
        pthread_mutex_lock(&buffer_mutex[process->id]);
        received_message_t *data = receiveMessage(process->id, buffer);
        if (data)
        {
            printf("Initiator %d:\t  Received message from %d\n", process->id, data->src);
            process->rec++;
            free(data);
        }
        pthread_mutex_unlock(&buffer_mutex[process->id]);
    }

    // decide
    printf("Initiator %d:\t  Deciding\n", process->id);
    return 0;
}

// non-initiator function
void *nonInitiator(void *arg)
{
    threadargs_t *args = (threadargs_t *)arg;
    process_t *process = args->process;
    pthread_mutex_t *buffer_mutex = args->buffer_mutex;
    int *flags = args->flags;
    pthread_mutex_t *flags_mutex = args->flags_mutex;
    message_t **buffer = args->buffer;

    // receive the first message and set the parent
    printf("Non-initiator %d:  Waiting for first message\n", process->id);
    pthread_mutex_lock(&buffer_mutex[process->id]);
    received_message_t *data = receiveMessage(process->id, buffer);
    if (data)
    {
        printf("Non-initiator %d:  Received message from parent %d\n", process->id, data->src);
        process->parent = data->src;
        process->rec++;
        free(data);
    }
    pthread_mutex_unlock(&buffer_mutex[process->id]);

    int sent_count = 0;
    // send the message to all neighbors except the parent
    while (sent_count < process->neigh_size)
    {
        if (process->neigh->dest != process->parent)
        {
            printf("Non-initiator %d:  Sending message to %d\n", process->id, process->neigh->dest);
            addMessage(process->id, process->neigh->dest, "message", buffer, buffer_mutex, flags, flags_mutex);
        }
        process->neigh = process->neigh->next;
        sent_count++;
    }

    // receive messages from all neighbors except the parent
    while (process->rec < process->neigh_size)
    {
        pthread_mutex_lock(&buffer_mutex[process->id]);
        received_message_t *data = receiveMessage(process->id, buffer);
        if (data)
        {
            printf("Non-initiator %d:  Received message from %d\n", process->id, data->src);
            process->rec++;
            free(data);
        }
        pthread_mutex_unlock(&buffer_mutex[process->id]);
    }

    // send the message to the parent
    printf("Non-initiator %d:  Sending message to parent %d\n", process->id, process->parent);
    addMessage(process->id, process->parent, "p1", buffer, buffer_mutex, flags, flags_mutex);

    // end
    printf("Non-initiator %d:  Ending\n", process->id);

    return 0;
}

void echo(Graph *graph, int initiator_id)
{
    pthread_t threads[graph->V];
    process_t processes[graph->V];

    //create the buffer for the messages
    message_t **buffer = (message_t **)malloc(graph->V * sizeof(message_t *));
    for (int i = 0; i < graph->V; i++)
    {
        buffer[i] = NULL;
    }

    int flags[graph->V];
    for (int i = 0; i < graph->V; i++)
    {
        flags[i] = 0;
    }

    pthread_mutex_t flags_mutex = PTHREAD_MUTEX_INITIALIZER;    

    // define and initialize the mutexes
    pthread_mutex_t buffer_mutex[graph->V];
    for (int i = 0; i < graph->V; i++)
    {
        int e = pthread_mutex_init(&buffer_mutex[i], NULL);
        if (e != 0)
        {
            fprintf(stderr, "pthread_mutex_init(): %s\n", strerror(e));
            exit(EXIT_FAILURE);
        }
        pthread_mutex_lock(&buffer_mutex[i]);
    }

    

    for (int i = 1; i < graph->V; i++)
    {
        processes[i].id = i;
        processes[i].parent = 0;
        processes[i].rec = 0;
        processes[i].neigh = graph->adjList[i].head;
        processes[i].neigh_size = graph->adjList[i].size;

        threadargs_t *args = (threadargs_t *)malloc(sizeof(threadargs_t));
        args->process = &processes[i];
        args->buffer_mutex = buffer_mutex;
        args->buffer = buffer;
        args->flags = flags;
        args->flags_mutex = &flags_mutex;

        if (i == initiator_id)
        {
            pthread_create(&threads[i], NULL, initiator, args);
        }
        else
        {
            pthread_create(&threads[i], NULL, nonInitiator, args);
        }
    }

    // Join the threads
    for (int i = 1; i < graph->V; i++)
    {
        pthread_join(threads[i], NULL);
    }
    printf("\n");
    // print the rec and parent values
    for (int i = 1; i < graph->V; i++)
    {
        printf("Node %d: rec = %d, parent = %d\n", i, processes[i].rec, processes[i].parent);
    }
}
