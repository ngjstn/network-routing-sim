# network-routing-sim

### Simulation for distance-vector and link-state network routing algorithms 


## Graph-Network Topology
The network of routers is represented similarly to an adjacency list structure, with routers acting as graph nodes and neighbours representing the links between adjacent nodes. Rather than using an array to access a given router node, a linked list is used instead. This helps with dynamically sizing a given amount of routers, and allows for input router IDs to be non-sequential, out-of-order. So our adjacency list is basically a linked list of `router` structs, which connect to adjacent nodes represented as a linked list of `neighbour_entry` structs. 

Each router has a routing table representation given as a linked list of `table_entry` structs. When a message is being sent across the network from source to destination, the router will view it's table to determine the next hop along the shortest path. The hop itself is done by accessing the corresponding `neighbour_entry`. The method of populating these tables depends on the protocol being used. 

## Link-State 
Link-State in this simulation relies on dijkstra's algorithm to calculate the shortest path from one router to all other routers in the network. The network topology is known and provided during the shortest path computation to simulate the protocol's "global view" of the network. 

Basically whenever a change occurs, our graph network topology is used to update the routing table entries of a router. Since we use the graph directly, routers will have an updated understanding of the new topology. 

## Distance-Vector
The routing tables for each router are setup with only the entries for direct neighbours (adjacent nodes) that they're linked to. The simulation assumes that vector advertisements are immediately sent to all neighbours after this occurs. 

To reach initial routing table convergence, we've trivially picked a starting router to receive these first ads. When the first router receives, it kicks off a recursive algorithm to update it's own table entries with the advertised vectors, and advertise any of it's new changes to it's own neighbours. This recursively repeats until there are no more new changes made in the routing tables. The explained convergence sequence is also started after a change is made from the changeFile. 

Router advertisement vectors are interchangeably used as `neighbour_entry` structs for this protocol. 

## Build
The executables were built using the `Makefile` in a Windows Subsystem for Linux (WSL) running Ubuntu 22.04.4 LTS. Two executables will be created for link-state and distance-vector simulations, respectively: 
```
./lsr <topologyFile> <messageFile> <changesFile> # link state routing
./dvr <topologyFile> <messageFile> <changesFile> # distance vector routing
```