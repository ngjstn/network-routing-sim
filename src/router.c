#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <sys/types.h>
#include "../lib/router.h"
#include "../lib/minHeap.h"

// bfs to find shortest paths from src to dest 
// int find_shortest_path(router* router_list, router* src, int dest)
// {
//     if (src == NULL)
//     {
//         fprintf(stderr, "Error: Source router is NULL\n");
//         exit(1);
//     }

//     if (router_list == NULL)
//     {
//         fprintf(stderr, "Error: Router list is empty\n");
//         exit(1);
//     }

//     int queue[100]; 
//     int visited[100];
//     int front = 0;
//     int rear = 0; 

//     // initialize visited array to 0
//     for (int i = 0; i < 100; i++)
//     {
//         visited[i] = 0;
//     }

//     // add src to queue and mark as visited
//     visited[src->id] = 1;    
//     queue[rear++] = src->id;

//     while (front != rear)
//     {
//         // dequeue a router
//         int current = queue[front++]; 
//         fprintf(stdout, "dequeue: %d\n", current);

//         router* current_router = get_router(current, router_list);
//         fprintf(stdout, "current_router: %p, id: %d\n", current_router, current_router->id);

//         neighbour_entry* neighbour = current_router->neighbour_list;
//         while (neighbour != NULL)
//         {   
//             // found destination
//             if (neighbour->id == dest)
//             {
//                 // do stuff to reverse traverse the path
//                 fprintf(stdout, "found destination: %d\n", dest);
//                 return neighbour->path_cost;
//             }
//             // enqueue neighbour if not visited
//             if (!visited[neighbour->id])
//             {
//                 visited[neighbour->id] = 1;
//                 queue[rear++] = neighbour->id;
//                 fprintf(stdout, "enqueue: %d\n", neighbour->id);
//             }
//             neighbour = neighbour->next;
//         }
//     }
//     return -1;
// }

void djikstras(router* router_list, router* src)
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

    fprintf(stdout, "DJIKSTRAS on src router %d\n", src->id);

    // initialize min heap
    min_heap* heap = create_min_heap(num_routers);

    current = router_list;
    int i = 0;
    while (current != NULL)
    {
        current->dist_idx = i;
        dist[i] = INT_MAX;
        heap->array[i] = create_min_heap_node(current->id, dist[i]);
        current->pos_idx = i;
        heap->pos[i] = i;
        i++; 
        fprintf(stdout, "router id: %d assigned dist_idx: %d\n", current->id, current->dist_idx);
        current = current->next;
    }

    // set distance to source as 0 and update heap
    heap->array[num_routers-1] = create_min_heap_node(src->id, dist[num_routers-1]);
    heap->pos[num_routers-1] = num_routers-1;
    get_router(src->id, router_list)->dist_idx = num_routers-1;
    dist[num_routers-1] = 0;
    decrease_key(heap, num_routers-1, dist[num_routers-1]);
    heap->size = num_routers;

    fprintf(stdout, "heap size: %d\n", heap->size);

    // heap contains all routers with undetermined shortest distance from src
    while (!is_empty(heap))
    {
        min_heap_node* heap_node = extract_min(heap);
        fprintf(stdout, "\nextracted min: %d\n", heap_node->v);
        int extracted = heap_node->v;
        int ext_dist = get_router(extracted, router_list)->dist_idx;

        // traverse all adjacent routers of u
        neighbour_entry* neighbour = get_router(extracted, router_list)->neighbour_list;
        while (neighbour != NULL)
        {
            fprintf(stdout, "\nneighbour of router %d: %d\n", heap_node->v, neighbour->id);
            int neighbour_dist_idx = neighbour->router_neighbour->dist_idx; 
            fprintf(stdout, "dist_idx of neighbour: %d\n", neighbour_dist_idx);
            fprintf(stdout, "dist[ext_dist]: %d, neighbour->path_cost: %d, dist[v]: %d\n", dist[ext_dist], neighbour->path_cost, dist[neighbour_dist_idx]);
            fprintf(stdout, "is_in_heap: %d\n", is_in_heap(heap, neighbour_dist_idx));

            if (!is_in_heap(heap, neighbour_dist_idx) && dist[ext_dist] != INT_MAX && (neighbour->path_cost + dist[ext_dist]) < dist[neighbour_dist_idx])
            {
                dist[neighbour_dist_idx] = dist[ext_dist] + neighbour->path_cost; 
                fprintf(stdout, "updated dist[%d]: %d\n", neighbour_dist_idx, dist[neighbour_dist_idx]);
                decrease_key(heap, neighbour_dist_idx, dist[neighbour_dist_idx]);
            }
            neighbour = neighbour->next;
        }
    }

    // // initialize heap nodes and distance array values
    // for (int i = 0; i < num_routers; i++)
    // {
    //     dist[i] = INT_MAX; 
    //     heap->array[i] = create_min_heap_node(i, dist[i]);
    //     heap->pos[i] = i;
    // }

    // // set distance to source as 0 and update heap
    // heap->array[src->id] = create_min_heap_node(src->id, dist[src->id]);
    // heap->pos[src->id] = src->id;
    // dist[src->id] = 0;
    // decrease_key(heap, src->id, dist[src->id]);
    // heap->size = num_routers;

    // while (!is_empty(heap))
    // {
    //     min_heap_node* heap_node = extract_min(heap);
    //     int extracted = heap_node->v;

    //     // traverse all adjacent routers of u
    //     neighbour_entry* neighbour = get_router(extracted, router_list)->neighbour_list;
    //     while (neighbour != NULL)
    //     {
    //         int v = extracted; 
    //         if (!is_in_heap(heap, v) && dist[extracted] != INT_MAX && (neighbour->path_cost + dist[extracted]) < dist[neighbour->id])
    //         {
    //             dist[neighbour->id] = dist[extracted] + neighbour->path_cost; 
    //             decrease_key(heap, neighbour->id, dist[neighbour->id]);
    //         }
    //     }
    // }

    fprintf(stdout, "Vertex \t\t Distance from Source\n"); 
    current = router_list;
    while (current != NULL)
    {
        fprintf(stdout, "%d \t\t %d\n", current->id, dist[current->dist_idx]);
        current = current->next;
    }

    // printArr(dist, num_routers);
}


// used for creating a table entry struct
table_entry* create_table_entry(int dest, int next_hop, int path_cost)
{
    table_entry* entry = (table_entry*)malloc(sizeof(table_entry));
    if (entry == NULL)
    {
        fprintf(stderr, "Error: Unable to allocate memory for new table entry\n");
        exit(1);
    }
    entry->dest = dest;
    entry->next_hop = next_hop;
    entry->path_cost = path_cost;
    // entry->ttl = ttl;
    entry->next = NULL;

    return entry;
}

// adds a table entry to the routing table in ascending order of destination id
void add_table_entry(router* src_router, int dest, int next_hop, int path_cost)
{
    table_entry* new_entry = create_table_entry(dest, next_hop, path_cost);
    table_entry* current = src_router->table_head;
    table_entry* prev = NULL;

    // empty table
    if (current == NULL)
    {
        src_router->table_head = new_entry;
        return;
    }

    // insert at the front of the list
    if (dest < current->dest)
    {
        new_entry->next = current;
        src_router->table_head = new_entry;
        return;
    }

    // insert in the middle or end of the list
    while (current != NULL)
    {
        if (dest < current->dest)
        {
            prev->next = new_entry;
            new_entry->next = current;
            return;
        }
        prev = current;
        current = current->next;
    }

    // insert at the end of the list
    prev->next = new_entry;
}

// used for creating the entry struct, not for setting the neighbour link
neighbour_entry* create_neighbour_entry(router* router, int id, int path_cost)
{
    neighbour_entry* neighbour = (neighbour_entry*)malloc(sizeof(neighbour_entry));
    if (neighbour == NULL)
    {
        fprintf(stderr, "Error: Unable to allocate memory for new neighbour\n");
        exit(1);
    }
    neighbour->id = id;
    neighbour->path_cost = path_cost;
    neighbour->router_neighbour = router;
    neighbour->next = NULL;

    return neighbour;
}

// makes the 2 routers neighbours of each other; used in initial topology setup
void set_neighbour_link(router* router1, router* router2, int id1, int id2, int path_cost)
{
    neighbour_entry* neighbour_id1 = create_neighbour_entry(router1, id1, path_cost);
    neighbour_entry* neighbour_id2 = create_neighbour_entry(router2, id2, path_cost);

    // set the neighbour entries to the front of the neighbour list
    neighbour_entry* temp1 = router1->neighbour_list;
    router1->neighbour_list = neighbour_id2;
    neighbour_id2->next = temp1;
    fprintf(stdout, "added neighbour id:%d to router:%d\n", router1->neighbour_list->id, router1->id);

    neighbour_entry* temp2 = router2->neighbour_list;
    router2->neighbour_list = neighbour_id1;
    neighbour_id1->next = temp2;
    fprintf(stdout, "added neighbour id:%d to router:%d\n", router2->neighbour_list->id, router2->id);

    // update routing tables 
    // table_entry* router1_entry = create_table_entry(router1->id, router1->id, path_cost);
    // table_entry* router2_entry = create_table_entry(router2->id, router2->id, path_cost);
}

// used to get a router pointer by id 
router* get_router(int id, router* router_list)
{
    if (router_list == NULL)
    {
        fprintf(stderr, "Error: Router list is empty\n");
        exit(1);
    }

    router* current = router_list;
    while (current != NULL)
    {
        if (current->id == id)
        {
            return current;
        }
        current = current->next;
    }
    fprintf(stderr, "Warning: Router with id %d not found\n", id);
    return NULL;
}

router* create_router(int id)
{
    router* new_router = (router*)malloc(sizeof(router));
    if (new_router == NULL)
    {
        fprintf(stderr, "Error: Unable to allocate memory for new router\n");
        exit(1);
    }
    new_router->id = id;

    // new_router->table = (routing_table*)malloc(sizeof(routing_table));
    // if (new_router->table == NULL)
    // {
    //     fprintf(stderr, "Error: Unable to allocate memory for new router's routing table\n");
    //     free(new_router);
    //     exit(1);
    // }
    // new_router->table->head = NULL;

    new_router->table_head = NULL;
    new_router->neighbour_list = NULL;
    new_router->next = NULL;
    return new_router;
}

// initializes the router linked list with the routers and their neighbours
router* init_routers(char* topologyFile)
{
    FILE* file = fopen(topologyFile, "r");
    char* line = NULL;
    size_t len = 0;
    size_t read;
    router* router_list = NULL;

    if (file == NULL)
    {
        fprintf(stderr, "Error: File %s not found\n", topologyFile);
        exit(1);
    }

    // each line in the topologyFile is a link between 2 routers
    fprintf(stdout, "CREATING ROUTER TOPOLOGY\n");
    while ((read = getline(&line, &len, file)) != -1)
    {
        char* id1 = strtok(line, " "); 
        char* id2 = strtok(NULL, " ");
        char* path_cost = strtok(NULL, " ");
        printf("LINE -> id1: %s, id2: %s, path_cost: %s\n", id1, id2, path_cost);

        // router list is empty
        if (router_list == NULL)
        {
            // create a router for id1 and set it to router_list (head of linked list)
            router* router1 = create_router(atoi(id1)); 
            fprintf(stdout, "created router %p, id: %d\n", router1, router1->id);
            router* router2 = create_router(atoi(id2));
            fprintf(stdout, "created router %p, id: %d\n\n", router2, router2->id);
            set_neighbour_link(router1, router2, atoi(id1), atoi(id2), atoi(path_cost));

            // set router1 as the head of the list; place router2 after router1 (end of linked list)
            router2->next = router1; 
            router_list = router2;
        }
        else 
        {
            int found1 = 0; 
            int found2 = 0; 
            router* router1; 
            router* router2; 
            router* current = router_list; 
            // see if router with id1 and id2 already exists
            while (current != NULL)
            {
                // found router; set local pointer and flag for reference
                if (current->id == atoi(id1))
                {
                    router1 = current;
                    found1 = 1;
                }
                else if (current->id == atoi(id2))
                {
                    router2 = current;
                    found2 = 1;
                }

                // found both routers, no need to continue searching 
                if (found1 && found2)
                {
                    break;
                }
                current = current->next;
            }

            // create a router for id1 and/or id2 if not found earlier and place at the front of the list
            current = router_list;
            if (!found1)
            {
                router1 = create_router(atoi(id1));
                router1->next = current;
                router_list = router1;
                fprintf(stdout, "created router %p, id: %d\n", router1, router1->id);
            }
            if (!found2)
            {
                router2 = create_router(atoi(id2));
                router2->next = current;
                router_list = router2;
                fprintf(stdout, "created router %p, id: %d\n", router2, router2->id);
            }
            
            // fprintf(stdout, "router1: %p, router2: %p\n", router1, router2);
            // fprintf(stdout, "router1->id: %d, router2->id: %d\n", router1->id, router2->id);
            // routers should be found or created by this point
            if (router1 == NULL || router2 == NULL)
            {
                fprintf(stderr, "Error: Unable to initialize topology graph\n");
                exit(1);
            }
            set_neighbour_link(router1, router2, atoi(id1), atoi(id2), atoi(path_cost));
            // fprintf(stdout, "\n");
        }
    }
    fclose(file);

    // // print out router linked list
    // fprintf(stdout, "\n\nINITIAL ROUTER LIST TOPOLOGY\n");
    // router* current = router_list;
    // while (current != NULL)
    // {
    //     fprintf(stdout, "router id: %d\n", current->id);

    //     // print out neighbour list 
    //     neighbour_entry* neighbour = current->neighbour_list;
    //     while (neighbour != NULL)
    //     {
    //         fprintf(stdout, "---> neighbour id: %d, path_cost: %d\n", neighbour->id, neighbour->path_cost);
    //         neighbour = neighbour->next;
    //     }
    //     current = current->next;
    // }

    fprintf(stdout, "router_list: %p\n", router_list);
    return router_list;
}