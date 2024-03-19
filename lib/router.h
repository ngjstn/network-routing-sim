#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include "../lib/minHeap.h"

#ifndef ROUTER_H
#define ROUTER_H

typedef struct table_entry
{
    int dest; 
    int next_hop; 
    int path_cost; 
    // int ttl; 
    struct table_entry* next; 
} table_entry;

// typedef struct routing_table 
// {
//     table_entry* head; 
//     int num_entries; 
// } routing_table;

typedef struct router
{
    int id; 
    int dist_idx; // distance from source (used to index dist array in djikstras alg.)
    int pos_idx; 
    table_entry* table_head;
    // routing_table* table; 
    struct neighbour_entry* neighbour_list;    // used for traversing the graph
    struct router* next;                       // used for linked list of ALL routers; doesn't represent neighbours
} router;

typedef struct neighbour_entry
{
    int id; 
    int path_cost;
    router* router_neighbour;
    struct neighbour_entry* next;
} neighbour_entry;

void djikstras(router* router_list, router* src);
// int find_shortest_path(router* router_list, router* src, int dest);
table_entry* create_table_entry(int dest, int next_hop, int path_cost); 
void add_table_entry(router* src_router, int dest, int next_hop, int path_cost); 
neighbour_entry* create_neighbour_entry(router* router, int id, int path_cost); 
void set_neighbour_link(router* router1, router* router2, int id1, int id2, int path_cost); 
router* get_router(int id, router* router_list);
router* create_router(int id);
router* init_routers(char* topologyFile);


#endif