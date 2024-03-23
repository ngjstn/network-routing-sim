#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include "../lib/minHeap.h"

#ifndef ROUTER_H
#define ROUTER_H

// used interchangeably as a vector for dv protocol
typedef struct table_entry
{
    int dest; 
    int next_hop; 
    int path_cost; 
    struct table_entry* next; 
} table_entry;

// graph node for the router network
typedef struct router
{
    int id; 
    int dist_idx;                              // distance from source (used to index dist array in bellman ford and dijkstras)
    table_entry* table_head;
    struct neighbour_entry* neighbour_list;    // used for traversing the graph through directly connected routers
    struct router* next;                       // used for link-state dijkstras global network view; NOT used/exposed in the distance-vector core logic 
} router;

// node for neighbour linked list stored in each router; represents an adjacent node in the graph
typedef struct neighbour_entry
{
    int id; 
    int path_cost;
    router* router_neighbour;                  // the actual router struct representing this neighbour
    struct neighbour_entry* next;              // next neighbour in the list 
} neighbour_entry;


// router/graph functions 
table_entry* create_table_entry(int dest, int next_hop, int path_cost); 
table_entry* get_routing_table_next_hop(router* src_router, int dest);
router* get_router(int id, router* router_list);
router* add_router(router* router_list, router* new_router);
router* create_router(int id);
router* init_routers(char* topologyFile);
router* get_neighbour(router* router, int id); 
void add_table_entry(router* src_router, int dest, int next_hop, int path_cost); 
void set_neighbour_link(router* router1, router* router2, int id1, int id2, int path_cost); 
void destroy_all_routing_tables(router* router_list);
void remove_neighbour_entry(router* router, int remove_id);

// modified heap functions (used in dijkstras)
min_heap_node* extract_min(min_heap* minHeap, router* router_list); 
void heapify(min_heap* minHeap, int idx, router* router_list); 
void decrease_key(min_heap* minHeap, int v, int dist, router* router_list);

// output file writing functions 
void write_tables_output(router* routing_list, char* outputFile);
void write_message_output(char *outputFile, char *message); 
void send_message(char* messageFile, char* outputFile, router* router_list);

#endif