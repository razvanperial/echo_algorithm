#include "graph.h"
#include <pthread.h>

#ifndef ECHO_H
#define ECHO_H

typedef struct Process
{
    int id;
    int parent;
    int rec;
    Node *neigh;
    int neigh_size;
} process_t;

typedef struct Message
{
    char *data;
    int src;
    struct Message *next;
} message_t;

typedef struct ReceivedMessage
{
    char *data;
    int src;
} received_message_t;

typedef struct ThreadArgs
{
    process_t *process;
    pthread_mutex_t* buffer_mutex;
    int *flags;
    pthread_mutex_t *flags_mutex;
    message_t **buffer;
} threadargs_t;

// function to add a message to the buffer
void addMessage(int src, int dest, char *data, message_t **buffer, pthread_mutex_t *buffer_mutex, int *flags, pthread_mutex_t *flags_mutex);

// function to receive a message from the buffer
received_message_t *receiveMessage(int dest, message_t **buffer);

// initiator function
void *initiator(void *arg);

// non-initiator function
void *nonInitiator(void *arg);

// function for running the echo algorithm on a multithreaded environment
void echo(Graph *graph, int initiator_id);

#endif // ECHO_H