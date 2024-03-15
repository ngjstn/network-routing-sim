#include <stdio.h>
#include <stdlib.h>
#include <string.h>


void link_state(char* topologyFile, char* messageFile, char* changesFile, char* outputFile)
{
    (void)topologyFile;
    (void)messageFile;
    (void)changesFile;
    (void)outputFile;
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