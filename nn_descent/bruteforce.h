#include "vec.h"
#include "knnd.h"

void nn_brute_force(float(*metric)(float*, float*, int), dataset_t data, update_t* updates, vec_t *vec_a, vec_t *vec_b);
void brute_force_new(float(*metric)(float*, float*, int), dataset_t data, update_t* updates, vec_t *vec_a);
void brute_force_new_old(float(*metric)(float*, float*, int), dataset_t data, update_t* updates, vec_t *vec_a, vec_t *vec_b);
void brute_force_new_unblocked(float(*metric)(float*, float*, int), dataset_t data, update_t* updates, vec_t *vec_a);
