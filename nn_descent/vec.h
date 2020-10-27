#pragma once
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    // one element i in heap has
    // - id ids[i]
    // - val (distance to other node) vals[i]
    // - isnews (flag whether this node is new neighbor) isnews[i]
    // they're in flat arrays to allow future vectorization

    int size; // current number of nodes in arr
    uint32_t *ids;
    float *vals;
    bool *isnews;
    int rev_new;
    int rev_old;
    int fwd_new;
    int fwd_old;
} heap_t;

typedef struct {
    int size;
    uint32_t *ids;
} vec_t;
void vec_insert_bounded(vec_t* h, uint32_t id, int max_candidates);
void vec_sort(vec_t* h);

int heap_create(heap_t*, int);
void heap_clear();
void heap_free(heap_t*);



void max_heapify(heap_t* h, int i);
int heap_insert_bounded(heap_t* h, uint32_t id, float dist, bool isnew, int max_neighbors);
int heap_find_by_index(heap_t*, uint32_t);

int array_find_by_index(uint32_t* arr, uint32_t index, int size);
