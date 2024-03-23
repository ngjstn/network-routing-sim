#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <sys/types.h>
#include "../lib/router.h"


// used to generate shortest path routing table entries for all routers in the network
void dijkstras(router* router_list, router* src)
{
    // find number of routers
    int num_routers = 0;
    router* current = router_list;
    while (current != NULL)
    {
        num_routers++;
        current = current->next;
    }

    // initialize distance array
    int* dist = (int*)malloc(num_routers * sizeof(int));
    if (dist == NULL)
    {
        fprintf(stderr, "Error: Unable to allocate memory for distance array\n");
        exit(1);
    }

    int* prev = (int*)malloc(num_routers * sizeof(int));
    if (prev == NULL)
    {
        fprintf(stderr, "Error: Unable to allocate memory for prev array\n");
        exit(1);
    }

    fprintf(stdout, "\nDIJKSTRAS on src router %d\n", src->id);

    // min heap for unprocessed router distances
    min_heap* heap = create_min_heap(num_routers);

    // initialize heap and array values to prep for dijkstras 
    current = router_list;
    int i = 0;
    while (current != NULL)
    {
        current->dist_idx = i;
        heap->pos[i] = i;

        // src router starts with 0 distance and is the root node of the min heap
        if (current->id == src->id)
        {
            prev[i] = src->id; 
            dist[i] = 0; 
            heap->array[i] = create_min_heap_node(current->id, dist[i]);
            decrease_key(heap, i, dist[i], router_list);
        }
        // all other routers start with infinite distance 
        else 
        {
            dist[i] = INT_MAX;
            prev[i] = -1;
            heap->array[i] = create_min_heap_node(current->id, dist[i]);
        }
        i++; 
        current = current->next;
    }
    heap->size = num_routers;

    // heap contains all routers with undetermined shortest distance from src
    while (!is_empty(heap))
    {
        min_heap_node* heap_node = extract_min(heap, router_list);
        // fprintf(stdout, "\nextracted min: %d\n", heap_node->v);
        int extracted = heap_node->v;
        int ext_dist = get_router(extracted, router_list)->dist_idx;

        // traverse all adjacent routers of u
        neighbour_entry* neighbour = get_router(extracted, router_list)->neighbour_list;
        while (neighbour != NULL)
        {
            int neighbour_dist_idx = neighbour->router_neighbour->dist_idx; 
            if (!is_in_heap(heap, neighbour_dist_idx) && dist[ext_dist] != INT_MAX && (neighbour->path_cost + dist[ext_dist]) <= dist[neighbour_dist_idx])
            {
                if (neighbour->router_neighbour->id > extracted && neighbour->path_cost + dist[ext_dist] == dist[neighbour_dist_idx])
                {
                    prev[neighbour_dist_idx] = extracted;
                }
                else 
                {
                    dist[neighbour_dist_idx] = dist[ext_dist] + neighbour->path_cost; 
                    prev[neighbour_dist_idx] = heap_node->v;
                }
                decrease_key(heap, neighbour_dist_idx, dist[neighbour_dist_idx], router_list);
            }
            neighbour = neighbour->next;
        }
    }

    // populate routing table entries for src router 
    fprintf(stdout, "ROUTER %d TABLE\n", src->id);
    fprintf(stdout, "Dest \t\t Next \t\t Distance from Source\n"); 
    current = router_list;
    while (current != NULL)
    {
        // reverse traverse the path from dest to src to find next hop from src
        router* temp = current;
        while (prev[temp->dist_idx] != src->id)
        {
            temp = get_router(prev[temp->dist_idx], router_list); 
        }
        // create route entry and add to src router's table
        fprintf(stdout, "%d \t\t %d \t\t %d\n", current->id, temp->id, dist[current->dist_idx]);
        add_table_entry(src, current->id, temp->id, dist[current->dist_idx]);
        current = current->next;
    }

    // dealloc arrays after data is updated into routing table entries
    free(dist);
    free(prev);
}

void process_change_topology(char* messageFile, char* changesFile, char* outputFile, router* router_list)
{
    FILE* file = fopen(changesFile, "r"); 
    char* line = NULL;
    size_t len = 0;
    size_t read;
    if (file == NULL)
    {
        fprintf(stderr, "Error: File %s not found\n", changesFile);
        exit(1);
    }

    while ((read = getline(&line, &len, file)) != -1)
    {
        // parse the line 
        int src_id = atoi(strtok(line, " "));
        int dest_id = atoi(strtok(NULL, " "));
        int cost = atoi(strtok(NULL, "\n"));

        destroy_all_routing_tables(router_list);
 
        router* src_router = get_router(src_id, router_list);
        if (src_router == NULL)
        {
            // router does not exist in the graph; add it
            src_router = create_router(src_id); 
            add_router(router_list, src_router);
        }

        router* dest_router = get_router(dest_id, router_list);
        if (dest_router == NULL)
        {
            // router does not exist in the graph; add it
            dest_router = create_router(dest_id); 
            add_router(router_list, dest_router);
        }

        // removing link between src and dest 
        if (cost == -999)
        {
            fprintf(stdout, "Removing neighbour link between %d and %d\n", src_id, dest_id);
            // remove neighbour link 
            remove_neighbour_entry(src_router, dest_id); 
            remove_neighbour_entry(dest_router, src_id); 
        }
        // add/change link between src and dest
        else 
        {
            // neighbour link already exists 
            if (get_neighbour(src_router, dest_id) != NULL)
            {
                fprintf(stdout, "Updating neighbour link between %d and %d\n", src_id, dest_id);
                // remove old neighbour link
                remove_neighbour_entry(src_router, dest_id);
                remove_neighbour_entry(dest_router, src_id);

                // create new neighbour link 
                set_neighbour_link(src_router, dest_router, src_id, dest_id, cost);
            }
            // neighbour link doesn't exist
            else 
            {
                fprintf(stdout, "Adding neighbour link between %d and %d\n", src_id, dest_id);
                // create new neighbour link 
                set_neighbour_link(src_router, dest_router, src_id, dest_id, cost);
            }
        }

        // run dijkstra's algorithm on all routers to update tables
        router* current = router_list;
        while (current != NULL)
        {
            dijkstras(router_list, current);
            current = current->next;
        }

        // write to output file after convergence
        write_tables_output(router_list, outputFile); 
        send_message(messageFile, outputFile, router_list);
    }
}

void link_state(char* topologyFile, char* messageFile, char* changesFile, char* outputFile)
{
    // establish graph nodes and links (initial state)
    // the graph itself represents any routers' global view of the network topology
    router* graph = init_routers(topologyFile);

    // determine shortest path route from one router to every other router in the network 
    router* current = graph;
    while (current != NULL)
    {
        // route calculation and table entry updates
        dijkstras(graph, current);
        current = current->next;
    }

    // update output file after convergence
    write_tables_output(graph, outputFile);
    send_message(messageFile, outputFile, graph);

    // begin applying changes to the graph topology
    process_change_topology(messageFile, changesFile, outputFile, graph);
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