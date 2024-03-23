/*
 * The following code related to the min heap structure was adopted from the geeksforgeeks website
 * source: https://www.geeksforgeeks.org/dijkstras-algorithm-for-adjacency-list-representation-greedy-algo-8/
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#ifndef MINHEAP_H
#define MINHEAP_H

typedef struct min_heap_node
{
    int v;
    int dist;
} min_heap_node;

typedef struct min_heap
{
    int size; 
    int capacity;
    int *pos;
    min_heap_node **array;
} min_heap;


min_heap* create_min_heap(int capacity);
min_heap_node* create_min_heap_node(int v, int dist);
void swap_heap_node(min_heap_node **a, min_heap_node **b);
int is_empty(min_heap* heap);
int is_in_heap(min_heap* heap, int v);

#endif 