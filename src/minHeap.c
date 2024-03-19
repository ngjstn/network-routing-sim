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

void heapify(min_heap* minHeap, int idx)
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
        minHeap->pos[smallestNode->v] = idx;
        minHeap->pos[idxNode->v] = smallest;
 
        // Swap nodes
        swap_heap_node(&minHeap->array[smallest], 
                         &minHeap->array[idx]);
 
        heapify(minHeap, smallest);
    }
}

int is_empty(min_heap* heap)
{
    return heap->size == 0;
}

min_heap_node* extract_min(min_heap* minHeap)
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
    minHeap->pos[root->v] = minHeap->size-1;
    minHeap->pos[lastNode->v] = 0;
 
    // Reduce heap size and heapify root
    --minHeap->size;
    heapify(minHeap, 0);
 
    return root;
}

void decrease_key(min_heap* minHeap, int v, int dist)
{
    // Get the index of v in  heap array
    int i = minHeap->pos[v];
 
    // Get the node and update its dist value
    minHeap->array[i]->dist = dist;
 
    // Travel up while the complete 
    // tree is not heapified.
    // This is a O(Logn) loop
    while (i && minHeap->array[i]->dist < 
           minHeap->array[(i - 1) / 2]->dist)
    {
        // Swap this node with its parent
        minHeap->pos[minHeap->array[i]->v] = 
                                      (i-1)/2;
        minHeap->pos[minHeap->array[
                             (i-1)/2]->v] = i;
        swap_heap_node(&minHeap->array[i],  
                 &minHeap->array[(i - 1) / 2]);
 
        // move to parent index
        i = (i - 1) / 2;
    }
}

int is_in_heap(min_heap* heap, int v)
{
   if (heap->pos[v] < heap->size)
     return 0;
   return -1;
}

void printArr(int dist[], int n)
{
    fprintf(stdout, "Vertex \t\t Distance from Source\n");
    for (int i = 0; i < n; ++i)
        printf("%d \t\t %d\n", i, dist[i]);
}