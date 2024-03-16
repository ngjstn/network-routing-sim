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
    router* router_list = NULL; 
    init_routers(router_list, topologyFile);
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
