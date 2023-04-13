#include "graph.h"
#include <stdio.h>

// Function to create a new node in the adjacency list
Node* createNode(int dest) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    newNode->dest = dest;
    newNode->next = NULL;
    return newNode;
}

// Function to create a new graph with V vertices
Graph* createGraph(int V) {
    Graph* graph = (Graph*)malloc(sizeof(Graph));
    graph->V = V;
    graph->adjList = (List*)malloc(V * sizeof(List));
    for (int i = 1; i <= V; i++) {
        graph->adjList[i].head = NULL;
        graph->adjList[i].size = 0;
    }
    return graph;
}

// Function to add an edge to the graph
void addEdge(Graph* graph, int src, int dest) {
    // Add edge from src to dest
    Node* newNode = createNode(dest);
    newNode->next = graph->adjList[src].head;
    graph->adjList[src].head = newNode;
    // Add edge from dest to src
    newNode = createNode(src);
    newNode->next = graph->adjList[dest].head;
    graph->adjList[dest].head = newNode;
    graph->adjList[src].size++;
    graph->adjList[dest].size++;
}

// Function to create a new graph, specific problem 3.2 from the hw
Graph* populateGraph_1() {
    // Create a graph and add edges to it, hardcoded for the values in the problem
    Graph* graph = createGraph(8);

    addEdge(graph, 1, 2);
    addEdge(graph, 1, 6);
    addEdge(graph, 1, 7);
    addEdge(graph, 2, 3);
    addEdge(graph, 2, 4);
    addEdge(graph, 3, 4);
    addEdge(graph, 4, 5);
    addEdge(graph, 5, 6);
    addEdge(graph, 5, 7);
    addEdge(graph, 6, 7);

    return graph;
}

Graph* populateGraph_2() {
    // Create a graph and add edges to it, hardcoded for the value in the slides
    // Note: value of the edges are incremented by 1compared to the slides, since 
    // the array starts at 1, with the initiator being 1
    Graph* graph = createGraph(6);

    addEdge(graph, 1, 2);
    addEdge(graph, 1, 3);
    addEdge(graph, 1, 4);
    addEdge(graph, 2, 3);
    addEdge(graph, 2, 4);
    addEdge(graph, 2, 5);
    addEdge(graph, 4, 5);

    return graph;
}
