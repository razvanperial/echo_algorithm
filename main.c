// Implementation of the Echo Algorithm, using multiple threads

#include <stdio.h>
#include "echo.h"

int main()
{
    // Create the graph
    Graph *graph_1 = populateGraph_1();
    Graph *graph_2 = populateGraph_2();

    // Run the echo algorithms
    echo(graph_1, 1);
    printf("\n-----------------Starting graph 2------------------\n\n");
    echo(graph_2, 1);

    return 0;
}