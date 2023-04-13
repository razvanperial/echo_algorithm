// Implementation of the Echo Algorithm, using multiple threads

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include "graph.h"

typedef struct Process
{
    int id; // for debugging purposes
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

// mutex initialization for the message passing
static pthread_mutex_t message[8];

// flags that mark the sending of the first message
static int first[8] = {0, 0, 0, 0, 0, 0, 0, 0};

// mutex for the flags variable
static pthread_mutex_t first_mutex = PTHREAD_MUTEX_INITIALIZER;

// initialize buffers for the string messages
static message_t *buffer[8];

// function to add a message to the buffer
void addMessage(int src, int dest, char *data)
{
    message_t *newMessage = (message_t *)malloc(sizeof(message_t));
    newMessage->data = (char *)malloc(strlen(data) + 1); // Allocate memory for data
    strcpy(newMessage->data, data);                      // Copy data to new memory
    newMessage->next = NULL;
    newMessage->src = src;

    pthread_mutex_lock(&first_mutex);
    if (first[dest] == 0)
    {
        first[dest] = 1;
        buffer[dest] = newMessage;
        newMessage->next = NULL;
        pthread_mutex_unlock(&message[dest]);
    }
    else
    {
        pthread_mutex_lock(&message[dest]);

        newMessage->next = buffer[dest];
        buffer[dest] = newMessage;
        pthread_mutex_unlock(&message[dest]);
    }
    pthread_mutex_unlock(&first_mutex);
}

// function to receive a message from the buffer
received_message_t *receiveMessage(int dest)
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
    process_t *process = (process_t *)arg;


    // send messages to all neighbors
    Node *temp = process->neigh;
    Node *iter = process->neigh;
    while (iter)
    {
        printf("Initiator %d:\t  Sending message to %d\n", process->id, iter->dest);
        addMessage(process->id, iter->dest, "p1");
        iter = iter->next;
    }
    process->neigh = temp;

    // receive messages from all neighbors
    while (process->rec < process->neigh_size)
    {
        // printf("Initiator %d:\t  Waiting for message\n", process->id);
        pthread_mutex_lock(&message[process->id]);
        received_message_t *data = receiveMessage(process->id);
        if (data)
        {
            printf("Initiator %d:\t  Received message from %d\n", process->id, data->src);
            process->rec++;
            free(data);
        }
        pthread_mutex_unlock(&message[process->id]);
    }

    // decide
    printf("Initiator %d:\t  Deciding\n", process->id);
    return 0;
}

// non-initiator function
void *nonInitiator(void *arg)
{
    process_t *process = (process_t *)arg;

    // receive the first message and set the parent
    printf("Non-initiator %d:  Waiting for first message\n", process->id);
    pthread_mutex_lock(&message[process->id]);
    received_message_t *data = receiveMessage(process->id);
    if (data)
    {
        printf("Non-initiator %d:  Received message from parent %d\n", process->id, data->src);
        process->parent = data->src;
        process->rec++;
        free(data);
    }
    pthread_mutex_unlock(&message[process->id]);

    int sent_count = 0;
    // send the message to all neighbors except the parent
    while (sent_count < process->neigh_size)
    {
        if (process->neigh->dest != process->parent)
        {
            printf("Non-initiator %d:  Sending message to %d\n", process->id, process->neigh->dest);
            addMessage(process->id, process->neigh->dest, "message");
        }
        process->neigh = process->neigh->next;
        sent_count++;
    }

    // receive messages from all neighbors except the parent
    while (process->rec < process->neigh_size)
    {
        pthread_mutex_lock(&message[process->id]);
        received_message_t *data = receiveMessage(process->id);
        if (data)
        {
            printf("Non-initiator %d:  Received message from %d\n", process->id, data->src);
            process->rec++;
            free(data);
        }
        pthread_mutex_unlock(&message[process->id]);
    }

    // send the message to the parent
    printf("Non-initiator %d:  Sending message to parent %d\n", process->id, process->parent);
    addMessage(process->id, process->parent, "p1");

    // end
    printf("Non-initiator %d:  Ending\n", process->id);

    return 0;
}

void echo(Graph *graph, int initiator_id)
{
    // Create the threads
    pthread_t threads[graph->V];
    process_t processes[graph->V];

    for (int i = 1; i < graph->V; i++)
    {
        processes[i].id = i;
        processes[i].parent = 0;
        processes[i].rec = 0;
        processes[i].neigh = graph->adjList[i].head;
        processes[i].neigh_size = graph->adjList[i].size;
        if (i == initiator_id)
        {
            pthread_create(&threads[i], NULL, initiator, &processes[i]);
        }
        else
        {
            pthread_create(&threads[i], NULL, nonInitiator, &processes[i]);
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

int main()
{

    // initialize the mutexes and lock all, such that the process that sends the first message will unlock it
    for (int i = 1; i <= 7; i++)
    {
        int e = pthread_mutex_init(&message[i], NULL);
        if (e)
        {
            fprintf(stderr, "pthread_mutex_init(): %s\n", strerror(e));
            exit(EXIT_FAILURE);
        }
        pthread_mutex_lock(&message[i]);
    }

    // Create the graph
    Graph *graph_1 = populateGraph_1();
    Graph *graph_2 = populateGraph_2();

    message_t *buffer_1[graph_1->V];
    message_t *buffer_2[graph_2->V];

    // Run the echo algorithms
    echo(graph_1, 1);
    // echo(graph_2, 1);

    return 0;
}
