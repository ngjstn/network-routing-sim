#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include "../lib/minHeap.h"

min_heap* create_min_heap(int capacity)
{
    min_heap* heap = (min_heap*)malloc(sizeof(min_heap));
    heap->pos = (int*)malloc(capacity * sizeof(int));
    heap->capacity = capacity;
    heap->size = 0;
    heap->array = (min_heap_node**)malloc(capacity * sizeof(min_heap_node*));
    return heap;
}

min_heap_node* create_min_heap_node(int v, int dist)
{
    min_heap_node* node = (min_heap_node*)malloc(sizeof(min_heap_node));
    if (node == NULL)
    {
        fprintf(stderr, "Error: Memory allocation failed for heap node\n");
        exit(1);
    }
    node->v = v;
    node->dist = dist;
    return node;
}

void swap_heap_node(min_heap_node** a, min_heap_node** b)
{
    min_heap_node* temp = *a;
    *a = *b;
    *b = temp;
}

int is_empty(min_heap* heap)
{
    return heap->size == 0;
}

int is_in_heap(min_heap* heap, int v)
{
   if (heap->pos[v] < heap->size)
     return 0;
   return -1;
}