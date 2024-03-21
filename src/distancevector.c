#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <sys/types.h>
#include "../lib/router.h"

// source router advertisement 
void bellman_ford(router* router_list, router* src)
{
    fprintf(stdout, "BELLMAN FORD\n"); 

    // find number of routers 
    int num_routers = 0;
    router* current = router_list;
    while (current != NULL)
    {
        num_routers++;
        current = current->next;
    }

    // initialize distance array 
    int* dist = (int*)malloc(sizeof(int) * num_routers);
    if (dist == NULL)
    {
        fprintf(stderr, "Error: Unable to allocate memory for distance array\n");
        exit(1);
    }

    // initialize next_hop array 
    int* next_hop = (int*)malloc(sizeof(int) * num_routers);
    if (next_hop == NULL)
    {
        fprintf(stderr, "Error: Unable to allocate memory for next hop array\n");
        exit(1);
    }

    // initialize array values 
    current = router_list;
    int i = 0;
    while (current != NULL)
    {
        current->dist_idx = i;
        if (current->id == src->id)
        {
            dist[i] = 0;
            next_hop[i] = -1; 
        }
        else 
        {
            dist[i] = INT_MAX;
            next_hop[i] = -1;
        }
        current = current->next;
        i++; 
    }

    // current = router_list; 
    // while (current != NULL)
    // {
    //     neighbour_entry* neighbour = current->neighbour_list; 
    //     while (neighbour != NULL)
    //     {
    //         if (neighbour->path_cost + dist[current->dist_idx] < dist[neighbour->router_neighbour->dist_idx])
    //         {
    //             dist[neighbour->router_neighbour->dist_idx] = neighbour->path_cost + dist[current->dist_idx];
    //             next_hop[neighbour->router_neighbour->dist_idx] = current->id;
    //         }
    //         neighbour = neighbour->next;
    //     }
    //     current = current->next;
    // }

    // current = router_list; 
    // while (current != NULL)
    // {
    //     neighbour_entry* neighbour = current->neighbour_list; 
    //     while (neighbour != NULL)
    //     {
    //         if (neighbour->path_cost + dist[neighbour->router_neighbour->dist_idx] < dist[current->dist_idx])
    //         {
    //             dist[current->dist_idx] = neighbour->path_cost + dist[neighbour->router_neighbour->dist_idx];
    //             next_hop[current->dist_idx] = neighbour->id;
    //         }
    //         neighbour = neighbour->next;
    //     }
    //     current = current->next;
    // }

    // update the routing table entries
    current = router_list; 
    while (current != NULL)
    {
        if (dist[current->dist_idx] != INT_MAX)
        {
            add_table_entry(src, current->id, next_hop[current->dist_idx], dist[current->dist_idx]);
        }
        current = current->next; 
    }
}

// router tables are initialized with only route entries for direct neighbours
void init_tables_entries(router* router_list)
{
    fprintf(stdout, "INIT TABLES\n");
    router* current_router = router_list; 
    while (current_router != NULL)
    {
        // add router path to itself 
        add_table_entry(current_router, current_router->id, current_router->id, 0);

        // add neighbour paths to the table
        neighbour_entry* current_neighbour = current_router->neighbour_list; 
        while (current_neighbour != NULL)
        {
            add_table_entry(current_router, current_neighbour->id, current_neighbour->id, current_neighbour->path_cost);
            current_neighbour = current_neighbour->next; 
        }
        current_router = current_router->next; 
    }
}

void distance_vector(char* topologyFile, char* messageFile, char* changesFile, char* outputFile)
{
    (void)topologyFile;
    (void)messageFile;
    (void)changesFile;
    (void)outputFile;

    router* router_list = init_routers(topologyFile);
    init_tables_entries(router_list);

    // router* current = router_list;
    // while (current != NULL) 
    // {
    //     bellman_ford(router_list, current);
    //     current = current->next;
    // }
    bellman_ford(router_list, get_router(1, router_list));
    write_tables_output(router_list, outputFile);
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