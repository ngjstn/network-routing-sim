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

void write_message_output(char *outputFile, char *message)
{
    FILE *file = fopen(outputFile, "a");
    if (file == NULL)
    {
        fprintf(stderr, "Error: File %s not found\n", outputFile);
        exit(1);
    }

    // write the message to the file
    fwrite(message, 1, strlen(message), file);
    fwrite("\n", 1, 1, file);
    fclose(file);
}

void send_message(char* messageFile, char* outputFile, router* router_list)
{
    FILE* file = fopen(messageFile, "r");
    char* line = NULL;
    size_t len = 0;
    size_t read;
    if (file == NULL)
    {
        fprintf(stderr, "Error: File %s not found\n", messageFile);
        exit(1);
    }

    // find number of routers 
    int num_routers = 0;
    router* current = router_list;
    while (current != NULL)
    {
        num_routers++;
        current = current->next;
    }
    
    while ((read = getline(&line, &len, file)) != -1)
    {
        // parse the line 
        int src = atoi(strtok(line, " ")); 
        int dest = atoi(strtok(NULL, " ")); 
        char* message = strtok(NULL, "\n");
        
        // initialize hop id values to -1
        int* hops = (int*)malloc(sizeof(int) * num_routers);
        for (int i = 0; i < num_routers; i++)
        {
            hops[i] = -1;
        }

        // grab the starting source router
        router* current = get_router(src, router_list);
        if (current == NULL)
        {
            fprintf(stderr, "Error: Message source router %d not found\n", src);
            exit(1);
        }
        int path_cost = -1; 

        // first hop is the source itself
        hops[0] = src; 
        int hop_idx = 1;
        
        // traverse to the destination router
        int unreachable = 0; 
        while (current->id != dest)
        {
            // search routing table for next hop id
            table_entry* tab_entry = get_routing_table_next_hop(current, dest); 
            if (tab_entry == NULL)
            {
                // this counts as destination unreachable
                fprintf(stderr, "Warning: No route from %d to %d\n", src, dest);
                unreachable = 1; 
                break;
            }
            
            // only update the path cost if it's the first hop
            if (path_cost == -1)
            {
                path_cost = tab_entry->path_cost; 
            }

            // traverse to next hop router
            current = get_neighbour(current, tab_entry->next_hop);
            if (current == NULL)
            {
                // this counts as destination unreachable 
                fprintf(stderr, "Warning: Next hop router %d not found\n", tab_entry->next_hop);
                unreachable = 1;
                break; 
            }
            
            // only need to record hops that aren't the destination 
            if (current->id != dest)
            {
                // add the next hop id to the hops array
                hops[hop_idx] = current->id;
                hop_idx++;
            }
        }

        fprintf(stdout, "from %d to %d cost %d hops", src, dest, path_cost);
        for (int i = 0; i < num_routers; i++)
        {
            if (hops[i] != -1)
            {
                fprintf(stdout, " %d", hops[i]);
            }
        }
        fprintf(stdout, " message %s\n", message);

        char* buffer = (char*)malloc(sizeof(char) * (strlen(message) + num_routers + 100));
        memset(buffer, '\0', sizeof(*buffer)); 
        if (unreachable)
        {
            sprintf(buffer, "from %d to %d cost infinite hops unreachable message %s\n", src, dest, message);
        }
        else
        {
            // replace buffer, src, dest, pathCost, hops array, message with proper values
            sprintf(buffer, "from %d to %d cost %d hops ", src, dest, path_cost);

            for (int i = 0; i < num_routers; i++)
            {
                if (hops[i] != -1)
                {
                    sprintf(buffer + strlen(buffer), "%d ", hops[i]);
                }
            }

            sprintf(buffer + strlen(buffer), "message %s", message);
        }

        // write to file
        write_message_output(outputFile, buffer);
    }
    
}

// updates the output file with ALL the routing tables in the routing_list 
void write_tables_output(router* routing_list, char* outputFile)
{
    FILE* file = fopen(outputFile, "a+");
    if (file == NULL)
    {
        fprintf(stderr, "Error: File %s not found\n", outputFile);
        exit(1);
    }

    router* curr_router = routing_list;
    while (curr_router != NULL)
    {
        if (curr_router->neighbour_list == NULL)
        {
            fprintf(stdout, "Warning: Router %d has no neighbours\n", curr_router->id);
        }

        // iterate through all the routes in the table
        table_entry* tab_entry = curr_router->table_head; 
        while (tab_entry != NULL)
        {
            char buffer[100]; 
            fprintf(stdout, "%d %d %d\n", tab_entry->dest, tab_entry->next_hop, tab_entry->path_cost);
            sprintf(buffer, "%d %d %d\n", tab_entry->dest, tab_entry->next_hop, tab_entry->path_cost);
            fwrite(buffer, 1, strlen(buffer), file);
            tab_entry = tab_entry->next;
        }
        fwrite("\n", 1, 1, file);
        fprintf(stdout, "\n");
        curr_router = curr_router->next;
    }
    fclose(file);
}

void distance_vector(char* topologyFile, char* messageFile, char* changesFile, char* outputFile)
{
    (void)messageFile;
    (void)changesFile;
    (void)outputFile;

    // list of all routers
    router* router_list = init_routers(topologyFile);
    router* current = router_list;
    current = router_list; 
    while (current != NULL)
    {
        djikstras(router_list, current);
        current = current->next;
    }

    // write tables for initial topology convergence 
    fprintf(stdout, "\nWRITING TABLES TO %s\n\n", outputFile); 
    write_tables_output(router_list, outputFile);


    
    fprintf(stdout, "WRITING MESSAGES TO %s\n\n", outputFile);
    send_message(messageFile, outputFile, router_list);
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
