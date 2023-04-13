
#ifndef GRAPH_H
#define GRAPH_H

#include <stdlib.h>

// Structure to represent a node in the adjacency list
typedef struct Node {
    int dest;           // destination vertex
    struct Node* next;  // pointer to next node in the list
} Node;

// Structure to represent the adjacency list
typedef struct List {
    Node* head;         // pointer to the first node in the list
    int size;           // size of the list
} List;

// Structure to represent the graph
typedef struct Graph {
    int V;              // number of vertices
    List* adjList;      // pointer to the adjacency list
} Graph;

// Function to create a new node in the adjacency list
Node* createNode(int dest);

// Function to create a new graph with V vertices
Graph* createGraph(int V);

// Function to add an edge to the graph
void addEdge(Graph* graph, int src, int dest);

// Function to create a new graph, specific problem 3.2 from the hw
Graph* populateGraph_1();

// Function to create a new graph, specific value in the slides
Graph* populateGraph_2();

#endif  // GRAPH_H
