#pragma once
#include "vec.h"
#include <stdbool.h>

// TODO: maybe use bit flags for bool argument in sample_neighbors?

typedef struct {
    float** values; // be sure to cast to float or double types
    int size; // number of points in the dataset
    int dim;
} dataset_t;

typedef struct {
    uint32_t* u;
    uint32_t* v;
    float* dist;
    int size;;
} update_t;

int make_test_data(dataset_t*, int, int);
heap_t* heap_list_create(int, int);
void heap_list_free(heap_t*, int);
heap_t* nn_descent(dataset_t, float(*)(float*, float*, int), int, float, float);
int sample_reverse_union(vec_t* new, vec_t* old, heap_t* B, int max_candidates, int N);
int update_nn(heap_t*, int, float);
int reverse_heap_list(heap_t*, heap_t*, int);
int validate_connection_counters(heap_t* B, int N);
void** reallocate_data(uint32_t* bwd_permutation, uint32_t* fwd_permutation, heap_t* B, dataset_t data, int k);
void switch_i_j(uint32_t* bwd_permutation, uint32_t* fwd_permutation, uint32_t i, uint32_t j);
int partition (float* vals, uint32_t* ids, int low, int high);
void quickSort(float* vals, uint32_t* ids, int low, int high);
