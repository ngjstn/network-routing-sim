#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

typedef struct table_entry
{
    int dest; 
    int next_hop; 
    int path_cost; 
    int ttl; 
    struct table_entry* next; 
} table_entry;

typedef struct routing_table 
{
    table_entry* head; 
    int num_entries; 
} routing_table;

typedef struct router
{
    int id; 
    routing_table* table; 
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


static void set_neighbour_link(router* router1, router* router2, int id1, int id2, int path_cost)
{
    // set the routers to be neighbours of each other 
    neighbour_entry* neighbour_id1 = (neighbour_entry*)malloc(sizeof(neighbour_entry));
    neighbour_entry* neighbour_id2 = (neighbour_entry*)malloc(sizeof(neighbour_entry));
    neighbour_id1->id = id2;
    neighbour_id2->id = id1;
    neighbour_id1->path_cost = path_cost;
    neighbour_id2->path_cost = path_cost;
    neighbour_id1->router_neighbour = router2;
    neighbour_id2->router_neighbour = router1;
    router1->neighbour_list = neighbour_id1; 
    router2->neighbour_list = neighbour_id2;
}

static router* get_router(int id, router* router_list)
{
    router* current = router_list;
    while (current != NULL)
    {
        if (current->id == id)
        {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

static router* create_router(int id)
{
    router* new_router = (router*)malloc(sizeof(router));
    if (new_router == NULL)
    {
        fprintf(stderr, "Error: Unable to allocate memory for new router\n");
        exit(1);
    }
    new_router->id = id;

    new_router->table = (routing_table*)malloc(sizeof(routing_table));
    if (new_router->table == NULL)
    {
        fprintf(stderr, "Error: Unable to allocate memory for new router's routing table\n");
        free(new_router);
        exit(1);
    }
    new_router->table->head = NULL;
    new_router->neighbour_list = NULL;
    new_router->next = NULL;
    return new_router;
}

void init_routers(router* router_list, char* topologyFile)
{
    FILE* file = fopen(topologyFile, "r");
    char* line = NULL;
    size_t len = 0;
    size_t read;

    if (file == NULL)
    {
        fprintf(stderr, "Error: File %s not found\n", topologyFile);
        exit(1);
    }

    // each line in the topologyFile is a link between 2 routers
    while ((read = getline(&line, &len, file)) != -1)
    {
        printf("%s\n", line);

        char* id1 = strtok(line, " "); 
        char* id2 = strtok(line, " ");
        char* path_cost = strtok(line, " ");
        printf("id1: %s, id2: %s, path_cost: %s\n", id1, id2, path_cost);

        // router list is empty
        if (router_list == NULL)
        {
            // create a router for id1 and set it to router_list (head of linked list)
            router* router1 = create_router(atoi(id1)); 
            router* router2 = create_router(atoi(id2));
            set_neighbour_link(router1, router2, atoi(id1), atoi(id2), atoi(path_cost));

            // set router1 as the head of the list; place router2 after router1 (end of linked list)
            router_list = router1;
            router_list->next = router2;
        }
        else 
        {
            int found1 = 0; 
            int found2 = 0; 
            router* router1; 
            router* router2; 
            router* current = router_list; 
            // see if router with id1 and id2 already exists
            while (current->next != NULL)
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
            }
            if (!found2)
            {
                router1 = create_router(atoi(id2));
                router1->next = current;
                router_list = router2;
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
}

void process_change_topology(char* changesFile, router* router_list)
{
    (void)changesFile;
    (void)router_list;
}

void send_message(char* messageFile, router* router_list)
{
    (void)messageFile;
    (void)router_list;
}

void write_output(char* outputFile, char* message)
{
    FILE* file = fopen(outputFile, "w");
    if (file == NULL)
    {
        fprintf(stderr, "Error: File %s not found\n", outputFile);
        exit(1);
    }

    // for output file

    // write to file in the following format
    // 1. cycle through topology entries (forwarding tables) by node id (in increasing order)
    // 2. cycle through message outputs (according to the order in the message file)
    // 3. repeat for each change in the changes file

    // workflow
    // write to file at beginning
    // apply one (1) change
    // write to char that gets put into file later ??
    // repeat for all remaining changes

    fclose(file);
}


void distance_vector(char* topologyFile, char* messageFile, char* changesFile, char* outputFile)
{
    // list of all routers
    router* router_list = NULL; 
    init_routers(router_list, topologyFile);
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
