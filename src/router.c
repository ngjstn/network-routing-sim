#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include "../lib/router.h"
#include "../lib/minHeap.h"

void heapify(min_heap* minHeap, int idx, router* router_list)
{
    int smallest, left, right;
    smallest = idx;
    left = 2 * idx + 1;
    right = 2 * idx + 2;
 
    if (left < minHeap->size && minHeap->array[left]->dist < minHeap->array[smallest]->dist)
    {
      smallest = left;
    }
 
    if (right < minHeap->size && minHeap->array[right]->dist < minHeap->array[smallest]->dist)
    {
      smallest = right;
    }
 
    if (smallest != idx)
    {
        // The nodes to be swapped in min heap
        min_heap_node *smallestNode = minHeap->array[smallest];
        min_heap_node *idxNode = minHeap->array[idx];
 
        // Swap positions
        minHeap->pos[get_router(smallestNode->v, router_list)->dist_idx] = idx;
        minHeap->pos[get_router(idxNode->v, router_list)->dist_idx] = smallest;
 
        // Swap nodes
        swap_heap_node(&minHeap->array[smallest], &minHeap->array[idx]);
 
        heapify(minHeap, smallest, router_list);
    }
}

min_heap_node* extract_min(min_heap* minHeap, router* router_list)
{
    if (is_empty(minHeap))
    {
        return NULL;
    }

    // Store the root node
    min_heap_node* root = minHeap->array[0];
 
    // Replace root node with last node
    min_heap_node* lastNode = minHeap->array[minHeap->size - 1];
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

    // get the node and update its dist value
    minHeap->array[i]->dist = dist;
 
    // travel up while the complete tree is not heapified.
    while (i && minHeap->array[i]->dist < minHeap->array[(i - 1) / 2]->dist)
    {
        // Swap this node with its parent
        minHeap->pos[get_router(minHeap->array[i]->v, router_list)->dist_idx] = (i-1)/2;
        minHeap->pos[get_router(minHeap->array[(i-1)/2]->v, router_list)->dist_idx] = i;
        swap_heap_node(&minHeap->array[i], &minHeap->array[(i - 1) / 2]);

        // move to parent index
        i = (i - 1) / 2;
    }
}

// deallocates all routing tables and its entries
void destroy_all_routing_tables(router* router_list)
{
    fprintf(stdout, "\nDEALLOCATING ROUTING TABLES\n");
    router* current = router_list; 
    while (current != NULL)
    {
        table_entry* current_entry = current->table_head;
        while (current_entry != NULL)
        {
            table_entry* temp = current_entry;
            current_entry = current_entry->next;
            free(temp);
        }
        current->table_head = NULL;
        current = current->next;
    }

    // verify all routing tables are deallocated
    current = router_list;
    while (current != NULL)
    {
        if (current->table_head != NULL)
        {
            fprintf(stderr, "Error: Routing table for router %d not deallocated\n", current->id);
            exit(1);
        }
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
    entry->next = NULL;

    return entry;
}

// returns the table entry for dest found in the src router's table 
table_entry* get_routing_table_next_hop(router* src_router, int dest)
{
    table_entry* current = src_router->table_head;
    while (current != NULL)
    {
        if (current->dest == dest)
        {
            return current;
        }
        current = current->next;
    }
    fprintf(stdout, "dest: %d not found in routing table\n", dest);
    return NULL; 
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
static neighbour_entry* create_neighbour_entry(router* router, int id, int path_cost)
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

// remove a neighbour entry node from router's neighbour linked list
void remove_neighbour_entry(router* router, int remove_id)
{
    // remove from neighbour linked list 
    neighbour_entry* current = router->neighbour_list;
    neighbour_entry* prev = NULL;
    while (current != NULL)
    {
        if (current->id == remove_id)
        {
            // remove from front 
            if (prev == NULL)
            {
                router->neighbour_list = current->next;
            }
            // remove from end 
            else if (current->next == NULL)
            {
                prev->next = NULL; 
            }
            // remove from middle 
            else 
            {
                prev->next = current->next;
            }
            free(current);
            return;
        }
        prev = current;
        current = current->next;
    }
}

// return the adjacent neighbour router pointer found by id
router* get_neighbour(router* router, int id)
{
    neighbour_entry* current = router->neighbour_list;
    while (current != NULL)
    {
        if (current->id == id)
        {
            return current->router_neighbour;
        }
        current = current->next;
    }
    fprintf(stdout, "neighbour id: %d not found\n", id);
    return NULL;
}

// adds a neighbour entry to the router's neighbour linked list in ascending order of id
void add_neighbour_entry(router* router, neighbour_entry* neighbour)
{
    neighbour_entry* current = router->neighbour_list;
    neighbour_entry* prev = NULL;

    // empty list
    if (current == NULL)
    {
        router->neighbour_list = neighbour;
        return;
    }

    // insert at the front of the list
    if (neighbour->id < current->id)
    {
        neighbour->next = current;
        router->neighbour_list = neighbour;
        return;
    }

    // insert in the middle or end of the list
    while (current != NULL)
    {
        if (neighbour->id < current->id)
        {
            prev->next = neighbour;
            neighbour->next = current;
            return;
        }
        prev = current;
        current = current->next;
    }

    // insert at the end of the list
    prev->next = neighbour;
}


// creates a traversable link between the 2 routers in the graph topology
void set_neighbour_link(router* router1, router* router2, int id1, int id2, int path_cost)
{
    neighbour_entry* neighbour_id1 = create_neighbour_entry(router1, id1, path_cost);
    neighbour_entry* neighbour_id2 = create_neighbour_entry(router2, id2, path_cost);

    add_neighbour_entry(router1, neighbour_id2);
    add_neighbour_entry(router2, neighbour_id1);
}

// returns a router found by id in the router graph topology 
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

// creates a new router struct with the given id
router* create_router(int id)
{
    router* new_router = (router*)malloc(sizeof(router));
    if (new_router == NULL)
    {
        fprintf(stderr, "Error: Unable to allocate memory for new router\n");
        exit(1);
    }
    new_router->id = id;
    new_router->table_head = NULL;
    new_router->neighbour_list = NULL;
    new_router->next = NULL;
    return new_router;
}

// adds a router to the routing list graph in ascending order of id
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

// initializes the router adjacency linked list with the routers and their linked neighbours
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
    while ((read = getline(&line, &len, file)) != -1)
    {
        char* id1 = strtok(line, " "); 
        char* id2 = strtok(NULL, " ");
        char* path_cost = strtok(NULL, " ");

        // router list is empty
        if (router_list == NULL)
        {
            // create a router for id1 and set it to router_list (head of linked list)
            router* router1 = create_router(atoi(id1)); 
            router* router2 = create_router(atoi(id2));
            set_neighbour_link(router1, router2, atoi(id1), atoi(id2), atoi(path_cost));

            // set router1 as the head of the list; place router2 after router1 (end of linked list)
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
                router_list = add_router(router_list, router1);
            }
            if (!found2)
            {
                router2 = create_router(atoi(id2));
                router_list = add_router(router_list, router2);
            }
            
            // routers should be found or created by this point
            if (router1 == NULL || router2 == NULL)
            {
                fprintf(stderr, "Error: Unable to initialize topology graph\n");
                exit(1);
            }
            set_neighbour_link(router1, router2, atoi(id1), atoi(id2), atoi(path_cost));
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

    return router_list;
}

// writes a message to the output file
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
    fwrite("\n", 1, 1, file);
    fclose(file);
}

// updates the output file with ALL the routing tables in the routing_list; call after convergence
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

// sends a message using only the routing table entries; call after convergence
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

        if (!unreachable)
        {
            fprintf(stdout, "from %d to %d cost %d hops", src, dest, path_cost);
            for (int i = 0; i < num_routers; i++)
            {
                if (hops[i] != -1)
                {
                    fprintf(stdout, " %d", hops[i]);
                }
            }
            fprintf(stdout, " message %s\n", message);
        }

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