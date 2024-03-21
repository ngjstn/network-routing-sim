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
    struct table_entry* next; 
} table_entry;

typedef struct router
{
    int id; 
    int dist_idx; // distance from source (used to index dist array in djikstras alg.)
    int pos_idx; 
    table_entry* table_head;
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


// modified heap functions
void heapify(min_heap* minHeap, int idx, router* router_list); 
min_heap_node* extract_min(min_heap* minHeap, router* router_list); 
void decrease_key(min_heap* minHeap, int v, int dist, router* router_list);

// router/graph functions 
void djikstras(router* router_list, router* src);
table_entry* create_table_entry(int dest, int next_hop, int path_cost); 
void add_table_entry(router* src_router, int dest, int next_hop, int path_cost); 
neighbour_entry* create_neighbour_entry(router* router, int id, int path_cost); 
void set_neighbour_link(router* router1, router* router2, int id1, int id2, int path_cost); 
router* get_router(int id, router* router_list);
router* add_router(router* router_list, router* new_router);
router* create_router(int id);
router* init_routers(char* topologyFile);
table_entry* get_routing_table_next_hop(router* src_router, int dest);
router* get_neighbour(router* router, int id); 
void destroy_all_routing_tables(router* router_list);
void remove_neighbour_entry(router* router, int remove_id);


#endif