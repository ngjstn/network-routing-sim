#include <stdio.h>
#include <stdlib.h>
#include <string.h>


void link_state(char* topologyFile, char* messageFile, char* changesFile, char* outputFile)
{
    (void)topologyFile;
    (void)messageFile;
    (void)changesFile;
    (void)outputFile;

    // will need dijkstra's algorithm to find the shortest path
    // will need to read the topology file and create a graph

    // each router can only store its next hop
    // all routers must agree on topology

    // if there are ties, break them the same way

    // routers periodically say hello to each other
    // no specific listening protocol
    // if they stop hearing from a router, they assume it's dead

    // flood information
    // tell everyone that you are neighbors
    // when local info changes, send to all neighbors
    // if you receive a packet from neighbour, send to all other neighbours
    //      unless you've already seen it
    //      for the most part this is related to reliability and drops
    //      idk if we need to worry about it

    // everyone floods link/destination information
    // everyone has a global map of the network
    // everyone independently computes next-hops

}


int main(int argc, char** argv)
{
    if (argc != 4 && argc != 5)
    {
        fprintf(stderr, "Usage: %s topologyFile messageFile changesFile outputFile\n\n", argv[0]);
        exit(1);
    }

    char* topologyFile = argv[1];
    char* messageFile = argv[2];
    char* changesFile = argv[3];
    char* outputFile = NULL;
    if (argc == 5)
    {
        outputFile = argv[4];
    }
    else 
    {
        outputFile = "output.txt";
    }

    link_state(topologyFile, messageFile, changesFile, outputFile);

    return EXIT_SUCCESS; 
}