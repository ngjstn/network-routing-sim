#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <sys/types.h>
#include "../lib/router.h"

// function prototypes for compiler
void advertise_change(router* src, table_entry* updated_src_vector); 
void compare_vector(router* src, table_entry* neighbour_vector, int neighbour_id, int cost_to_neighbour);

// (UNUSED) compute routing table entries for src router 
void bellman_ford(router* router_list, router* src)
{
    fprintf(stdout, "\nBELLMAN FORD on router %d\n", src->id); 

    // find number of routers 
    int num_routers = 0;
    router* current = router_list;
    while (current != NULL)
    {
        num_routers++;
        current = current->next;
    }

    // initialize distance from src array 
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
        // src distance to itself is 0
        if (current->id == src->id)
        {
            dist[i] = 0;
        }
        else 
        {
            dist[i] = INT_MAX;
        }
        next_hop[i] = -1; 
        current = current->next;
        i++; 
    }

    // exchange routing information 
    for (int i = 0; i < num_routers; i++)
    {
        router* current_router = router_list; 
        while (current_router != NULL)
        {
            neighbour_entry* current_neighbour = current_router->neighbour_list;
            while (current_neighbour != NULL)
            {
                if (dist[current_neighbour->router_neighbour->dist_idx] != INT_MAX && dist[current_neighbour->router_neighbour->dist_idx] + current_neighbour->path_cost < dist[current_router->dist_idx])
                {
                    dist[current_router->dist_idx] = dist[current_neighbour->router_neighbour->dist_idx] + current_neighbour->path_cost;
                    next_hop[current_router->dist_idx] = current_neighbour->id;
                }
                current_neighbour = current_neighbour->next;
            }
            current_router = current_router->next;
        }
    }

    // update the routing table entries
    fprintf(stdout, "Dest \t\t Next \t\t Distance from Source\n"); 
    current = router_list; 
    while (current != NULL)
    {
        fprintf(stdout, "%d \t\t %d \t\t %d\n", current->id, next_hop[current->dist_idx], dist[current->dist_idx]);
        current = current->next; 
    }

    free(dist);
    free(next_hop);
}

// router tables are initialized with only route entries for direct neighbours
void init_tables_entries(router* router_list)
{
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

// transmit the new vector change to all neighbours of the src; recursive for propogating vector changes
void advertise_change(router* src, table_entry* updated_src_vector)
{
    neighbour_entry* current_neighbour = src->neighbour_list;
    while (current_neighbour != NULL) 
    {
        // find the neighbour table entry that corresponds to the updated vector
        table_entry* current_vector = current_neighbour->router_neighbour->table_head;
        while (current_vector != NULL)
        {
            if (current_vector->dest == updated_src_vector->dest)
            {
                fprintf(stdout, "\nAdvertising to router %d\n", current_neighbour->id);
                compare_vector(current_neighbour->router_neighbour, updated_src_vector, src->id, current_neighbour->path_cost);
                break; 
            }
            current_vector = current_vector->next;
        }
        current_neighbour = current_neighbour->next;
    }
}

// determine if a better path is advertised to the src; if so, update the src routing table and recursively advertise the change to all neighbours
void compare_vector(router* src, table_entry* neighbour_vector, int neighbour_id, int cost_to_neighbour)
{
    // SPLIT HORIZON: should not advertise routes back to the router that sent it
    // in simulation, just don't accept routes from the neighbour that has the src as the next hop 
    if (neighbour_vector->dest == src->id)
    {
        // ignore this advertisement
        fprintf(stdout, "\nIgnoring route to router %d from router %d\n", neighbour_vector->dest, neighbour_id);
        return; 
    }

    fprintf(stdout, "\nComparing vector from router %d: dest %d, next hop %d, cost %d\n", neighbour_id, neighbour_vector->dest, neighbour_vector->next_hop, neighbour_vector->path_cost);

    // look for the destination in the src table 
    table_entry* current_vector = src->table_head; 
    while (current_vector != NULL)
    { 
        fprintf(stdout, "current route to %d\n", current_vector->dest);
        if (current_vector->dest == neighbour_vector->dest)
        {
            // neighbour is the current next hop to the dest 
            if (current_vector->next_hop == neighbour_id)
            {
                fprintf(stdout, "Old route to router %d from router %d with cost %d\n", neighbour_vector->dest, neighbour_id, neighbour_vector->path_cost + cost_to_neighbour);
                // replace current route with neighbours route 
                int old_cost = current_vector->path_cost;

                // POISON ad doesn't need to include the cost to the neighbour since it's infinite
                if (neighbour_vector->path_cost == INT_MAX)
                {
                    current_vector->path_cost = INT_MAX; 
                }
                // regular ad includes the cost to the neighbour
                else 
                {
                    current_vector->path_cost = neighbour_vector->path_cost + cost_to_neighbour;
                }
                fprintf(stdout, "New route to router %d from router %d with cost %d\n", neighbour_vector->dest, neighbour_id, neighbour_vector->path_cost + cost_to_neighbour);
                
                // only advertise if the cost has changed
                if (current_vector->path_cost != old_cost && current_vector->path_cost >= 0)
                {
                    advertise_change(src, current_vector);
                }
            }
            // neighbours' advertised distance is better than current distance to dest
            else if (neighbour_vector->path_cost != INT_MAX && neighbour_vector->path_cost + cost_to_neighbour < current_vector->path_cost)
            {
                fprintf(stdout, "Old route to router %d from router %d with cost %d\n", neighbour_vector->dest, neighbour_id, neighbour_vector->path_cost + cost_to_neighbour);
                current_vector->path_cost = neighbour_vector->path_cost + cost_to_neighbour;
                current_vector->next_hop = neighbour_id;
                fprintf(stdout, "Updating route to router %d from router %d with cost %d\n", neighbour_vector->dest, neighbour_id, neighbour_vector->path_cost + cost_to_neighbour);
                advertise_change(src, current_vector);
            }
            else 
            {
                fprintf(stdout, "No change to route to router %d from router %d with cost %d\n", neighbour_vector->dest, neighbour_id, neighbour_vector->path_cost + cost_to_neighbour);
            }
            return; 
        }
        current_vector = current_vector->next;
    }
    
    // dest not found in the src table, so add the new route  
    fprintf(stdout, "Adding new route to router %d from router %d with cost %d\n", neighbour_vector->dest, neighbour_id, neighbour_vector->path_cost + cost_to_neighbour);
    add_table_entry(src, neighbour_vector->dest, neighbour_id, neighbour_vector->path_cost + cost_to_neighbour);
    table_entry* new_src_vector = get_routing_table_next_hop(src, neighbour_vector->dest);
    advertise_change(src, new_src_vector);
    return; 
}

// simulates the src router receiving a message from a direct neighbour (assumes the messages are sent immediately after initial table setup)
void receive_pseudo_ads(router* src)
{
    // each neighbour sends its routing table entries to the src
    neighbour_entry* current_neighbour = src->neighbour_list;
    while (current_neighbour != NULL)
    {
        // process each vector entry from the neighbour
        table_entry* current_vector = current_neighbour->router_neighbour->table_head;  
        while (current_vector != NULL)
        {
            // compare with vectors in case we need to update the current route
            compare_vector(src, current_vector, current_neighbour->id, current_neighbour->path_cost);
            current_vector = current_vector->next;
        }
        current_neighbour = current_neighbour->next;
    }
}

// poison the hops that go through the updated link dest_id 
void link_update_ad(router* src_router, int dest_id, int new_cost)
{
    // poison src table entries that have next hop as dest
    fprintf(stdout, "\nPoisoning routes to router %d\n", dest_id);
    table_entry* current_vector = src_router->table_head;
    while (current_vector != NULL)
    {
        if (current_vector->next_hop == dest_id)
        {
            current_vector->path_cost = new_cost;
            advertise_change(src_router, current_vector);
            receive_pseudo_ads(src_router);
        }
        current_vector = current_vector->next;
    }
}

void process_change_topology(char* messageFile, char* changesFile, char* outputFile, router* router_list)
{
    FILE* file = fopen(changesFile, "r");
    char* line = NULL; 
    size_t len = 0;
    size_t read;
    if (file == NULL)
    {
        fprintf(stderr, "Error: Unable to open changes file\n");
        exit(1);
    }

    while ((read = getline(&line, &len, file)) != -1)
    {
        // parse the line 
        int src_id = atoi(strtok(line, " "));
        int dest_id = atoi(strtok(NULL, " "));
        int cost = atoi(strtok(NULL, "\n"));

        router* src_router = get_router(src_id, router_list);
        if (src_router == NULL)
        {
            // router does not exist in the graph; add it
            src_router = create_router(src_id); 
            add_router(router_list, src_router);
            add_table_entry(src_router, src_id, src_id, 0);
        }

        router* dest_router = get_router(dest_id, router_list);
        if (dest_router == NULL)
        {
            // router does not exist in the graph; add it
            dest_router = create_router(dest_id); 
            add_router(router_list, dest_router);
            add_table_entry(dest_router, dest_id, dest_id, 0);
        }

        // removing link between src and dest
        if (cost == -999)
        {
            fprintf(stdout, "Removing link between %d and %d\n", src_id, dest_id);
            // break graph link 
            remove_neighbour_entry(src_router, dest_id);
            remove_neighbour_entry(dest_router, src_id);

            // poison the hops that go through the broken link by setting the cost to infinity
            link_update_ad(src_router, dest_id, INT_MAX);
            link_update_ad(dest_router, src_id, INT_MAX);

            // receive new ads after poison to update routing table shortest paths 
            fprintf(stdout, "\n\nAFTER POISON ADVERTISEMENT\n");
            router* current = router_list; 
            while (current != NULL) 
            {
                receive_pseudo_ads(current);
                current = current->next;
            }
        }
        // add/change link between src and dest 
        else 
        {
            // neighbour link already exists
            if (get_neighbour(src_router, dest_id) != NULL)
            {
                fprintf(stdout, "Changing link between %d and %d\n", src_id, dest_id);
                remove_neighbour_entry(src_router, dest_id);
                remove_neighbour_entry(dest_router, src_id);
                set_neighbour_link(src_router, dest_router, src_id, dest_id, cost);
                
                // update next hop paths that were affected by the link change
                link_update_ad(src_router, dest_id, cost); 
                link_update_ad(dest_router, src_id, cost);
            }
            // neighbour link doesn't exist
            else
            {
                fprintf(stdout, "Adding link between %d and %d\n", src_id, dest_id);
                set_neighbour_link(src_router, dest_router, src_id, dest_id, cost); 

                // propogate the change to all neighbours of the src and dest routers 
                advertise_change(src_router, get_routing_table_next_hop(src_router, dest_id));
                advertise_change(dest_router, get_routing_table_next_hop(dest_router, src_id));
                // receive_pseudo_ads(src_router); 
                // receive_pseudo_ads(dest_router);
            }

            // receive new ads to update routing table shortest paths
            router* current = router_list; 
            while (current != NULL) 
            {
                receive_pseudo_ads(current);
                current = current->next;
            }
        }

        // update output file after convergence 
        write_tables_output(router_list, outputFile);
        send_message(messageFile, outputFile, router_list);
    }
}

void distance_vector(char* topologyFile, char* messageFile, char* changesFile, char* outputFile)
{
    // establish graph nodes and links 
    router* graph = init_routers(topologyFile);

    // initialize routing tables with only direct neighbour entries
    init_tables_entries(graph);
    
    // routing table changes begin by receiving initial vectors from neighbours
    // assumes that the neighbours advertise their vectors immediately after the tables are initialized
    router* current = graph; 
    while (current != NULL)
    {
        // begin propogating initial vectors to all neighbours
        receive_pseudo_ads(current);
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

    distance_vector(topologyFile, messageFile, changesFile, outputFile);
    return EXIT_SUCCESS; 
}