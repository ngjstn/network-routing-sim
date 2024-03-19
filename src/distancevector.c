#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include "../lib/router.h"

void process_change_topology(char* changesFile, router* router_list)
{
    (void)changesFile;
    (void)router_list;
}

void send_message(char* messageFile, router* router_list)
{
    (void)messageFile;
    (void)router_list;
}

void write_output(char* outputFile, char* message)
{
    (void)message;

    FILE* file = fopen(outputFile, "w");
    if (file == NULL)
    {
        fprintf(stderr, "Error: File %s not found\n", outputFile);
        exit(1);
    }

    // for output file

    // write to file in the following format
    // 1. cycle through topology entries (forwarding tables) by node id (in increasing order)
    // 2. cycle through message outputs (according to the order in the message file)
    // 3. repeat for each change in the changes file

    // workflow
    // write to file at beginning
    // apply one (1) change
    // write to char that gets put into file later ??
    // repeat for all remaining changes

    fclose(file);
}


void distance_vector(char* topologyFile, char* messageFile, char* changesFile, char* outputFile)
{
    (void)messageFile;
    (void)changesFile;
    (void)outputFile;

    // list of all routers
    router* router_list = init_routers(topologyFile);
    
    // print out router linked list
    fprintf(stdout, "\n\nINITIAL ROUTER LIST TOPOLOGY\n");
    router* current = router_list;
    while (current != NULL)
    {
        fprintf(stdout, "router id: %d\n", current->id);

        // print out neighbour list 
        neighbour_entry* neighbour = current->neighbour_list;
        while (neighbour != NULL)
        {
            fprintf(stdout, "---> neighbour id: %d, path_cost: %d\n", neighbour->id, neighbour->path_cost);
            neighbour = neighbour->next;
        }
        current = current->next;
    }

    fprintf(stdout, "router_list: %p\n\n", router_list);
    djikstras(router_list, get_router(1, router_list));
    // find_shortest_path(router_list, get_router(1, router_list), get_router(3, router_list)->id);
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

    distance_vector(topologyFile, messageFile, changesFile, outputFile);

    return EXIT_SUCCESS; 
}
