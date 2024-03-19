#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <sys/types.h>
#include "../lib/router.h"
#include "../lib/minHeap.h"

void heapify(min_heap* minHeap, int idx, router* router_list)
{
    int smallest, left, right;
    smallest = idx;
    left = 2 * idx + 1;
    right = 2 * idx + 2;
 
    if (left < minHeap->size &&
        minHeap->array[left]->dist < 
         minHeap->array[smallest]->dist )
      smallest = left;
 
    if (right < minHeap->size &&
        minHeap->array[right]->dist <
         minHeap->array[smallest]->dist )
      smallest = right;
 
    if (smallest != idx)
    {
        // The nodes to be swapped in min heap
        min_heap_node *smallestNode = 
             minHeap->array[smallest];
        min_heap_node *idxNode = 
                 minHeap->array[idx];
 
        // Swap positions
        minHeap->pos[get_router(smallestNode->v, router_list)->dist_idx] = idx;
        minHeap->pos[get_router(idxNode->v, router_list)->dist_idx] = smallest;
 
        // Swap nodes
        swap_heap_node(&minHeap->array[smallest], 
                         &minHeap->array[idx]);
 
        heapify(minHeap, smallest, router_list);
    }
}

min_heap_node* extract_min(min_heap* minHeap, router* router_list)
{
    if (is_empty(minHeap))
        return NULL;
 
    // Store the root node
    min_heap_node* root = 
                   minHeap->array[0];
 
    // Replace root node with last node
    min_heap_node* lastNode = 
         minHeap->array[minHeap->size - 1];
    minHeap->array[0] = lastNode;
 
    // Update position of last node
    minHeap->pos[get_router(root->v, router_list)->dist_idx] = minHeap->size-1;
    minHeap->pos[get_router(lastNode->v, router_list)->dist_idx] = 0;
 
    // Reduce heap size and heapify root
    --minHeap->size;
    heapify(minHeap, 0, router_list);
 
    return root;
}

void decrease_key(min_heap* minHeap, int v, int dist, router* router_list)
{
    // Get the index of v in  heap array
    int i = minHeap->pos[v];
    // fprintf(stdout, "i: %d\n", i);
    // fprintf(stdout, "v: %d\n", v);

    // Get the node and update its dist value
    minHeap->array[i]->dist = dist;
 
    // Travel up while the complete 
    // tree is not heapified.
    // This is a O(Logn) loop
    while (i && minHeap->array[i]->dist < 
           minHeap->array[(i - 1) / 2]->dist)
    {
        // fprintf(stdout, "swap 1: %d\n", minHeap->array[i]->v);
        // fprintf(stdout, "swap 2: %d\n", minHeap->array[(i-1)/2]->v);
        // Swap this node with its parent
        minHeap->pos[get_router(minHeap->array[i]->v, router_list)->dist_idx] = (i-1)/2;
        minHeap->pos[get_router(minHeap->array[(i-1)/2]->v, router_list)->dist_idx] = i;
        swap_heap_node(&minHeap->array[i],  
                 &minHeap->array[(i - 1) / 2]);

        // fprintf(stdout, "swap 1: %d\n", minHeap->array[i]->v);
        // fprintf(stdout, "swap 2: %d\n", minHeap->array[(i-1)/2]->v);

        // move to parent index
        i = (i - 1) / 2;
    }
}

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

    int* prev = (int*)malloc(num_routers * sizeof(int));
    if (prev == NULL)
    {
        fprintf(stderr, "Error: Unable to allocate memory for prev array\n");
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
        current->pos_idx = i;
        heap->pos[i] = i;

        if (current->id == src->id)
        {
            prev[i] = src->id; 
            dist[i] = 0; 
            heap->array[i] = create_min_heap_node(current->id, dist[i]);
            // fprintf(stdout, "heap->array[0]: %d\n", heap->array[0]->dist);
            // fprintf(stdout, "heap->array[%d]: %d\n", i, heap->array[i]->dist);
            // fprintf(stdout, "router index 0: %d\n", get_router(heap->array[0]->v, router_list)->id);
            decrease_key(heap, i, dist[i], router_list);
            // fprintf(stdout, "heap->array[0]: %d\n", heap->array[0]->dist);
            // fprintf(stdout, "heap->array[%d]: %d\n", i, heap->array[i]->dist);
            // fprintf(stdout, "router index 0: %d\n", get_router(heap->array[0]->v, router_list)->id);
        }
        else 
        {
            dist[i] = INT_MAX;
            heap->array[i] = create_min_heap_node(current->id, dist[i]);
        }

        i++; 
        // fprintf(stdout, "router id: %d assigned dist_idx: %d\n", current->id, current->dist_idx);
        current = current->next;
    }

    // print out heap array 
    // fprintf(stdout, "heap array: \n");
    // for (int i = 0; i < num_routers; i++)
    // {
    //     fprintf(stdout, "router id: %d, dist: %d\n", heap->array[i]->v, heap->array[i]->dist);
    // }

    // set distance to source as 0 and update heap
    // heap->array[num_routers-1] = create_min_heap_node(src->id, dist[num_routers-1]);
    // heap->pos[num_routers-1] = num_routers-1;
    // get_router(src->id, router_list)->dist_idx = num_routers-1;
    // dist[num_routers-1] = 0;
    // decrease_key(heap, num_routers-1, dist[num_routers-1]);
    heap->size = num_routers;

    // fprintf(stdout, "heap size: %d\n", heap->size);

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
            // fprintf(stdout, "\nneighbour of router %d: %d\n", heap_node->v, neighbour->id);
            int neighbour_dist_idx = neighbour->router_neighbour->dist_idx; 
            // fprintf(stdout, "dist_idx of neighbour: %d\n", neighbour_dist_idx);
            // fprintf(stdout, "dist[ext_dist]: %d, neighbour->path_cost: %d, dist[neighbour_dist_idx]: %d\n", dist[ext_dist], neighbour->path_cost, dist[neighbour_dist_idx]);
            // fprintf(stdout, "is_in_heap: %d\n", is_in_heap(heap, neighbour_dist_idx));

            if (!is_in_heap(heap, neighbour_dist_idx) && dist[ext_dist] != INT_MAX && (neighbour->path_cost + dist[ext_dist]) < dist[neighbour_dist_idx])
            {
                dist[neighbour_dist_idx] = dist[ext_dist] + neighbour->path_cost; 
                prev[neighbour_dist_idx] = heap_node->v;
                // fprintf(stdout, "updated dist[%d]: %d\n", neighbour_dist_idx, dist[neighbour_dist_idx]);
                decrease_key(heap, neighbour_dist_idx, dist[neighbour_dist_idx], router_list);
            }
            neighbour = neighbour->next;
        }
    }

    fprintf(stdout, "ROUTER %d TABLE\n", src->id);
    fprintf(stdout, "Dest \t\t Next \t\t Distance from Source\n"); 
    current = router_list;
    while (current != NULL)
    {
        router* temp = current;
        while (prev[temp->dist_idx] != src->id)
        {
            temp = get_router(prev[temp->dist_idx], router_list); 
        }
        fprintf(stdout, "%d \t\t %d \t\t %d\n", current->id, temp->id, dist[current->dist_idx]);

        // add route to routing table 
        add_table_entry(src, current->id, temp->id, dist[current->dist_idx]);
        current = current->next;
    }
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

// adds router to routing list in ascending order of id
router* add_router(router* router_list, router* new_router)
{
    router* current = router_list;
    router* prev = NULL;

    // empty list
    if (current == NULL)
    {
        router_list = new_router;
        return router_list;
    }

    // insert at the front of the list
    if (new_router->id < current->id)
    {
        new_router->next = current;
        router_list = new_router;
        return router_list;
    }

    // insert in the middle or end of the list
    while (current != NULL)
    {
        if (new_router->id < current->id)
        {
            prev->next = new_router;
            new_router->next = current;
            return router_list;
        }
        prev = current;
        current = current->next;
    }

    // insert at the end of the list
    prev->next = new_router;
    return router_list; 
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
            // router2->next = router1; 
            // router_list = router2;
            router_list = add_router(router_list, router1);
            router_list = add_router(router_list, router2);
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
                // router1->next = current;
                // router_list = router1;
                router_list = add_router(router_list, router1);
                fprintf(stdout, "created router %p, id: %d\n", router1, router1->id);
            }
            if (!found2)
            {
                router2 = create_router(atoi(id2));
                // router2->next = current;
                // router_list = router2;
                router_list = add_router(router_list, router2);
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

    // print out router linked list
    fprintf(stdout, "\n\nINITIAL ROUTER LIST TOPOLOGY\n");
    fprintf(stdout, "router_list: %p\n", router_list);
    router* current = router_list;
    while (current != NULL)
    {
        fprintf(stdout, "router id: %d\n", current->id);

        // print out neighbour list 
        neighbour_entry* neighbour = current->neighbour_list;
        while (neighbour != NULL)
        {
            fprintf(stdout, "---> neighbour id: %d, path_cost: %d\n", neighbour->id, neighbour->path_cost);
            neighbour = neighbour->next;
        }
        current = current->next;
    }

    // fprintf(stdout, "router_list: %p\n", router_list);
    return router_list;
}