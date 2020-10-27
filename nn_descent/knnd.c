#include "knnd.h"
#include "vec.h"
#include "bruteforce.h"

#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <math.h>
#include <stdbool.h>
#include <assert.h>
#include <sys/time.h>
#include <string.h>

#define FREQUENCY 2.7e9

#define SWAP(x, y, T) {T tmp = x; x = y; y = tmp; }

vec_t* vec_list_create(int size, int k)
{
    // Scalar replacement
    uint32_t tmp1 = sizeof(vec_t) * size;
    uint32_t tmp2 = sizeof(uint32_t) * k;

    vec_t* vl = malloc(tmp1 + tmp2 * size);
    for (int i = 0; i < size; i++) {
        vl[i].size = 0;
        vl[i].ids = (uint32_t*)((size_t)vl + tmp1 + i * tmp2);
    }
    return vl;
}


heap_t* heap_list_create(int size, int k)
{
    // create list of 'size' many max heaps

    heap_t* hl = malloc(sizeof(heap_t) * size);
    hl->size = size;
    if (!hl) return NULL;

    for (int i = 0; i < size; i++) {
        if (!heap_create(&hl[i], k)) return NULL;
    }

    return hl;
}

void vec_list_clear(vec_t* hl, int size) {
    // logically clears/empyies heap list
    for (int i = 0; i < size; i++) {
        hl[i].size = 0;
    }
}
void heap_list_clear(heap_t* hl, int size) {
    // logically clears/empyies heap list
    for (int i = 0; i < size; i++) {
        heap_clear(&hl[i]);
    }
}

void heap_list_free(heap_t* hl, int size)
{
    for (int i = 0; i < size; i++) {
        heap_free(&hl[i]);
    }

    free(hl);
}

int nn_update(heap_t* B, heap_t* h, uint32_t id, float dist)
{
    // insert node into maxheap h if it is more similar than the current root

    if (dist >= h->vals[0] || heap_find_by_index(h, id) >= 0) return 0;

    heap_t *R = &B[h->ids[0]];
    if (h->isnews[0]) {
        // if old max element in heap was already flagged new, the outward new count on h stays the same
        // we need to remove the incoming (reverse) new count on the element R it was previously pointing to
        // and increase it on B[id] where it is now pointing
        R->rev_new--;
        B[id].rev_new++;
    } else {
        // to be replaced is not new
        R->rev_old--;
        B[id].rev_new++;

        // replacing forward old with
        // forward new
        h->fwd_old--;
        h->fwd_new++;
    }


    h->vals[0] = dist;
    h->isnews[0] = true;
    h->ids[0] = id;

    max_heapify(h, 0);

    return 1;
}

heap_t* nn_descent(dataset_t data, float(*metric)(float*, float*, int), int k, float rho, float delta)
{
    // Implementation of Algorithm 2: NNDescentFull from the publication

    // metric: function returning disimilarity between two vectors of given dimension
    // rho: sample rate in (0,1] (2.5 Sampling)
    // delta: precision parameter, terminate if less than delta*K*N updates in an iteration
    int max_candidates = 50;
    int data_size = data.size;

    if (k >= data_size) {
        printf("error: neighborhood size must be less than dataset size\n");
        return NULL;
    }

    // create lists with the maxheaps for each datapoint
    heap_t* B     = heap_list_create(data_size, k);
    vec_t* old   = vec_list_create(data_size, max_candidates);
    vec_t* new   = vec_list_create(data_size, max_candidates);

    if (!B || !old || !new) {
        printf("error: failed to allocate one or more heap lists\n");
        if (B) heap_list_free(B, data_size);
        if (old) free(old);
        if (new) free(new);
        return NULL;
    }

    // initialize heap list B
    for (int i = 0; i < data_size; i++) {
        // sample
        for (int j = 0; j < k;) {
            uint32_t id = (int)rand() % data_size;
            if (heap_insert_bounded(&B[i], id, FLT_MAX, true, k) == 1) {
                j++;
                B[id].rev_new++;
                B[i].fwd_new++;
            }
        }
    }

    int c; // number of updates in this iteration
    int stop_iter = delta * data_size * k; // terminate if less than this many changes made

    const int UPD_MAX_SIZE = 8*16384;
    update_t updates;
    updates.size=0;
    updates.u = malloc(UPD_MAX_SIZE*sizeof(uint32_t));
    updates.v = malloc(UPD_MAX_SIZE*sizeof(uint32_t));
    updates.dist = malloc(UPD_MAX_SIZE*sizeof(float));

    int iter = 1;
    struct timeval start, end;

    uint32_t* bwd_permutation = malloc(data.size * sizeof(uint32_t));
    uint32_t* fwd_permutation = malloc(data.size * sizeof(uint32_t));
    // initialize forward and backwards permutation as identity
    for (uint32_t i = 0; i < data.size; i++) {
      bwd_permutation[i] = i;
      fwd_permutation[i] = i;
    }

    do {
        // time one iteration of the main loop
        // only for additional infomation
        gettimeofday(&start, NULL);

        vec_list_clear(old, data_size);
        vec_list_clear(new, data_size);

        sample_reverse_union(new, old, B, max_candidates, data_size);

        c = 0;
        for (int v = 0; v < data_size; v++) {
            //int v = fwd_permutation[j];
            //printf("index: %d\n", v);

            // brute force algorithm to solve KNN:
            // for new[v] x new[v]
            // for new[v] x old[v]
            // vec_sort(&old[v]);
            vec_sort(&new[v]);
            brute_force_new(metric, data, &updates, &new[v]);
            //brute_force_new_unblocked(metric, data, &updates, &new[v]);
            //brute_force_new_old(metric, data, &updates, &new[v], &old[v]);
            nn_brute_force(metric, data, &updates, &new[v], &old[v]);
            // nn_brute_force(metric, data, &updates, &new[v], &new[v]);
            
            

            // we will create at most m_c*m_c*2 updates in the next iteration
            // if theres enough space we do not need to perform the updates yet
            if (UPD_MAX_SIZE-updates.size < max_candidates*max_candidates*2) {
                for (int i=0; i<updates.size; i++) {
                    if (updates.u[i] == updates.v[i]) continue;
                    c += nn_update(B, &B[updates.u[i]], updates.v[i] , updates.dist[i]);
                    c += nn_update(B, &B[updates.v[i]], updates.u[i] , updates.dist[i]);
                }
                updates.size=0;
            }
            // N * max_c * 2 * max_c
        }
        for (int i=0; i<updates.size; i++) {
            if (updates.u[i] == updates.v[i]) continue;
            c += nn_update(B, &B[updates.u[i]], updates.v[i] , updates.dist[i]);
            c += nn_update(B, &B[updates.v[i]], updates.u[i] , updates.dist[i]);
        }
        updates.size=0;
       // printf("iteration complete: %d / %d\n", c, stop_iter);

       // using the result of the first iteration in order to reallocate the data for the following iterations
       if (iter==1){
         // the reallocated data is returned using two pointers:
         // we did not reallocate in place to avoid unneccessary memcpys
         void** ptrs = reallocate_data(bwd_permutation, fwd_permutation, B, data, k);
         B = ptrs[0];
         data = *(dataset_t*) ptrs[1];
         free(ptrs);
       }

       // end of one main loop iteation, print the timing information
       gettimeofday(&end, NULL);
       double timing = (double)((end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec)/1e6);
       printf("Iteration %d: %f seconds\n", iter, timing);

       // iteration counter (NEEDED for the reallocate data not just for timing)
       iter++;

    } while (c >= stop_iter);

    // assert(validate_connection_counters(B, data.size)==data.size*k*2);
    // printf("done, cleaning up...\n");


    // revert the permutation of the B[i]'s in order to validate the result on the correct indices
    heap_t* B_ = heap_list_create(data.size, k);
    for (int i = 0; i < data.size; i++) {
      // copy the value array (needed for the heap_check in knnd_test),
      // can be avoided if only the id's are needed to verify the result
      memcpy(B_[i].vals, B[fwd_permutation[i]].vals, sizeof(float)*k);

      // revert the size and most important the id's
      B_[i].size = B[fwd_permutation[i]].size;
      for (int j=0; j<B[fwd_permutation[i]].size; j++) {
          B_[i].ids[j] = bwd_permutation[B[fwd_permutation[i]].ids[j]];
      }
    }

    printf("NNDescent finished \n");

    free(old);
    free(new);
    free(updates.u);
    free(updates.v);
    free(updates.dist);
    free(bwd_permutation);
    free(fwd_permutation);
    heap_list_free(B, data.size);

    return B_;
}

int compare (const void * a, const void * b)
{
  return ( **(float**)a >= **(float**)b )? 1 : -1;
}

// heuristic approach for reordering
void** reallocate_data(uint32_t* bwd_permutation, uint32_t* fwd_permutation, heap_t* B, dataset_t data, int k){
  // first compute the forward and backwards permutation derived by the Heuristic
  // afterwards reorder (copy) the data by the derived permutation


  for (int i=0; i<data.size; i++) {

    // Sort the values of B[i].vals as well as the corresponding B[i].ids
    // in order to iterate in the right order through the actual nearest neighbor approximation
    // core idea: consider the best neighbor first
    // in order to use the qsort stdlib function we pass the pointers to the function
    // --> effectively sorting pointers accoding to their valies
    // and can reuse the pointer offsets to sort the id's in the same way as the actual values
    float** ptr = malloc(k * sizeof(float*));
    for (int j=0; j<k; j++)
        ptr[j] = &(B[bwd_permutation[i]].vals[j]);
    qsort(ptr, k, sizeof(float*), compare);


    float* tmp_vals = malloc(k * sizeof(float));
    for (int j=0; j<k; j++)
        tmp_vals[j] = *(ptr[j]);

    // change ids to match the new sorted values
    uint32_t* tmp_ids = malloc(k * sizeof(uint32_t));
    for (int j=0; j<k; j++)
        tmp_ids[j] = *(ptr[j]-B[bwd_permutation[i]].vals + B[bwd_permutation[i]].ids);


    // Heuristic approach to reoder data
    for (int j=0; j<B[i].size; j++) {
      // j-th smallest element in B[i]
      // switch one element or if the wanted element is at the right position break
      // and continue with the next B[i+1]

      // exactly at the spot where we want
      if (fwd_permutation[tmp_ids[j]] == i+1)
        break;
      // reorder only if the target index is in the "untouched" region
      if (fwd_permutation[tmp_ids[j]] > i+1){
        switch_i_j(bwd_permutation, fwd_permutation, (i+1), fwd_permutation[tmp_ids[j]]);
        break;
      }
    }
  }

  // reorder (copy) the data with the derived permutation

  // reorder data.values

  // ensure that everything is 4 byte aligned
  // might need to alloc some more space for the pointer part of data.values
    uint32_t n256 = data.size;
    if (data.size % 4 != 0)
    {
        n256 += + (4 - (data.size % 4));
    } 

  dataset_t* data_ = malloc(sizeof(dataset_t));
  data_->values = aligned_alloc(256, (sizeof(float*) * n256) + (data.size * data.dim * sizeof(float))); // malloc((sizeof(float*) * data.size) + (data.size * data.dim * sizeof(float)));
  data_->size = data.size;
  data_->dim = data.dim;
  for (int i = 0; i < data.size; i++) {
      // row pointer arithmetic (according to read data in knnd_test)
      data_->values[i] = (float*)(data_->values + n256) + i * data.dim;
      memcpy(data_->values[i], data.values[bwd_permutation[i]], sizeof(float)*data.dim);
  }

  // reorder B[i]'s
  heap_t* B_     = heap_list_create(data.size, k);
  for (int i = 0; i < data.size; i++) {
      memcpy(B_[i].vals, B[bwd_permutation[i]].vals, sizeof(float)*k);
      memcpy(B_[i].isnews, B[bwd_permutation[i]].isnews, sizeof(bool)*k);
      memcpy(&(B_[i].rev_new), &(B[bwd_permutation[i]].rev_new), sizeof(int)*4);
      assert(B_[i].fwd_old == B[bwd_permutation[i]].fwd_old);
      B_[i].size = B[bwd_permutation[i]].size;
      for (int j=0; j<B[bwd_permutation[i]].size; j++) {
          B_[i].ids[j] = fwd_permutation[B[bwd_permutation[i]].ids[j]];
      }
  }

  // return pointers to the reordered data
  void** ptrs = malloc(sizeof(void*)*2);
  ptrs[0] = B_;
  ptrs[1] = data_;
  return  ptrs;
}

inline void switch_i_j(uint32_t* bwd_permutation, uint32_t* fwd_permutation, uint32_t i, uint32_t j){
  SWAP(fwd_permutation[bwd_permutation[i]], fwd_permutation[bwd_permutation[j]], uint32_t);
  SWAP(bwd_permutation[i], bwd_permutation[j], uint32_t);
}

int sample_reverse_union(vec_t* new, vec_t* old, heap_t* B, int max_candidates, int N) {
    // this function samples, reverses and does the union all at once

    for (int i=0; i<N; i++) {
        heap_t heap = B[i];
        int u = i;

        for (int j=0; j<heap.size; j++) {
            bool isnew = heap.isnews[j];
            uint32_t v = heap.ids[j];
            heap_t heap_v = B[v];

            // depending on flag select new or old heaplist
            vec_t* heap_list = (isnew) ? new : old;

            // pr_u is the number of elements we can sample from for heap_list[u]
            // same story for v
            int pr_u = (isnew) ? (heap.fwd_new + heap.rev_new) :  (heap.fwd_old + heap.rev_old);
            int pr_v = (isnew) ? (heap_v.fwd_new + heap_v.rev_new) :  (heap_v.fwd_old + heap_v.rev_old);


            // corresponds to sampling with pr max_candidates/pr_u
            if (rand() % pr_u < max_candidates) {
                vec_insert_bounded(&heap_list[u], v, max_candidates);

                // if we sampled a new connection
                // we can now
                // 1. change the flag
                // 2. update our counters
                if (isnew) {
                    B[u].isnews[j] = false;
                    // by flagging B[u][v] = false
                    // we have one fewer flagged u -> v
                    // and one more not-flagged u->v
                    B[u].fwd_new--;
                    B[u].fwd_old++;

                    // by flagging B[u][v] = false
                    // we have one fewer flagged v -> u
                    // and one more not-flagged v->u
                    B[v].rev_new--;
                    B[v].rev_old++;
                }
            }

            if (rand() % pr_v < max_candidates) {
                vec_insert_bounded(&heap_list[v], u, max_candidates);

                int index = heap_find_by_index(&B[v], u);
                if (index >= 0 && B[v].isnews[index]) {
                    // same story with the flags as above
                    B[v].isnews[index] = false;
                    B[v].fwd_new--;
                    B[v].fwd_old++;

                    B[u].rev_new--;
                    B[u].rev_old++;
                }
            }
        }
    }
    return 0;
}

int validate_connection_counters(heap_t* B, int N) {
    // this checks whether all the counters are what they're supposed to be
    // obviously this is for testing and shouldnt be executed in "production" code
    int sum=0;
    int *rev_old = calloc(N,sizeof(int));
    int *rev_new = calloc(N,sizeof(int));
    int *fwd_new = calloc(N,sizeof(int));
    int *fwd_old = calloc(N,sizeof(int));

    // this function samples, reverses and does the union all at once
    for (int i=0; i<N; i++) {
        heap_t heap = B[i];
        int u = i;
        // printf("i: %d, sum: %d\n", i, (B[i].fwd_new + B[i].fwd_old));
        // assert(B[i].fwd_new + B[i].fwd_old == 20);
        sum += B[i].fwd_new + B[i].fwd_old + B[i].rev_new + B[i].rev_old;


        for (int j=0; j<heap.size; j++) {
            bool isnew = heap.isnews[j];
            uint32_t v = heap.ids[j];
            if (isnew) {
                fwd_new[i]++;
                rev_new[v]++;
            } else {
                fwd_old[i]++;
                rev_old[v]++;
            }
        }
    }
    for (int i=0; i<N; i++) {
        heap_t heap = B[i];
        int u = i;
        assert(rev_old[i] == heap.rev_old);
        assert(rev_new[i] == heap.rev_new);
        assert(fwd_new[i] == heap.fwd_new);
        assert(fwd_old[i] == heap.fwd_old);
    }
    free(rev_old);
    free(rev_new);
    free(fwd_old);
    free(fwd_new);
    return sum;
}

void validate_permutation_vectors(uint32_t* fwd_permutation, uint32_t* bwd_permutation, int size){
  // permute ranom and test if we get the identity permutation
  // by applying the backwards first and then the forward permutation
  for (int l=0; l<2000; l++) {
    uint32_t i = rand()% size;
    uint32_t j = rand()% size;
    if (i==j) continue;
    switch_i_j(bwd_permutation,fwd_permutation, i, j);
    for (int k=0; k< size; k++) {
      assert(fwd_permutation[bwd_permutation[k]]==k);
    }
    // printf("%d %d\n", i, j);
  }
}
