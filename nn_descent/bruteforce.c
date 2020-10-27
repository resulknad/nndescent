#include "knnd.h"
#include "vec.h"
#include "bruteforce.h"
#include <stdio.h>
#include <assert.h>
#include <immintrin.h>
#include <math.h>

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))
#ifdef INSTR

#pragma GCC push_options
#pragma GCC optimize ("O0")

void sim_eval() {
}
#pragma GCC pop_options

#define DIST_EVAL() sim_eval()
#else
#define DIST_EVAL() true
#endif

#define BLOCKSIZE 5
#define MAX_CANDIDATES 50
 
inline float single_l2(float* v1, float* v2, int d)
{
    float acc = 0.0f;
    float acc2 = 0.0f;
    float acc3 = 0.0f;
    float acc4 = 0.0f;
    int offset=d/4;
    for (int i = 0; i < offset; i++) {
        acc += (v1[i] - v2[i]) * (v1[i] - v2[i]);
        acc2 += (v1[offset+i] - v2[offset+i]) * (v1[offset+i] - v2[offset+i]);
        acc3 += (v1[2*offset+i] - v2[2*offset+i]) * (v1[2*offset+i] - v2[2*offset+i]);
        acc4 += (v1[3*offset+i] - v2[3*offset+i]) * (v1[3*offset+i] - v2[3*offset+i]);
    }
    for (int i = 4*offset; i < d; i++) {
        acc += (v1[i] - v2[i]) * (v1[i] - v2[i]);
    }

    return acc+acc2+acc3+acc4;
}
 
void brute_force_new(float(*metric)(float*, float*, int), dataset_t data, update_t* updates, vec_t *vec_a)
{
    int vec_a_size = vec_a->size;
    //printf("%d", vec_a_size);

    // if(vec_a_size % BLOCKSIZE != 0){
    // // if(vec_a_size < 5){
    //   nn_brute_force(metric, data, updates, vec_a, vec_a);
    //   return;
    // }

    // number of updates that will be performed
    int new_updates = vec_a_size * (vec_a_size - 1) / 2;

    float** data_values = data.values;
    int data_dim = data.dim;

    // unroll 5
    int blocks = vec_a_size / BLOCKSIZE;

    for(int outer = 0; outer < blocks; outer++){

        // only need 5 in the triangular part
        __m256 vector_parts[2* BLOCKSIZE]; 

        //only need 10 in the triangular part
        __m256 accs[BLOCKSIZE * BLOCKSIZE];
        
        // compute the triangular part

        // in the upper triangular part for 5x5 there are 5 (5-1) / 2 = 10 combinations
        for(int k = 0; k < BLOCKSIZE * BLOCKSIZE; k++){
            accs[k] = _mm256_setzero_ps();
        }

        uint32_t ind[2 * BLOCKSIZE];

        for(int k = 0; k < BLOCKSIZE;k++){
            ind[k] = vec_a->ids[outer* BLOCKSIZE + k];
        }

        for(int l = 0; l < data_dim; l+= 8){
            for (int k = 0; k < BLOCKSIZE; k++){
                vector_parts[k] = _mm256_loadu_ps(&(data_values[ind[k]][l]));
            }

            int array_position = 0;
            for(int low = 0; low < BLOCKSIZE - 1; low++){
                for(int high = low + 1; high < BLOCKSIZE; high++){
                    __m256 difference = _mm256_sub_ps(vector_parts[low], vector_parts[high]);
                    // accs[array_position] = _mm256_fmadd_ps(difference, difference, accs[array_position]);
                    accs[low * BLOCKSIZE + high] = _mm256_fmadd_ps(difference, difference, accs[low * BLOCKSIZE + high]);
                    array_position++;
                }
            }    
        }

        int updates_base = updates->size;
        int array_position = 0;
        for(int low = 0; low < BLOCKSIZE - 1; low++){
            for(int high = low + 1; high < BLOCKSIZE; high++){
                float total_squared_l2 = 0;
                for(int k = 0; k < 8; k++){
                    // total_squared_l2 += accs[array_position][k];
                    total_squared_l2 += accs[low * BLOCKSIZE + high][k];
                }
                //int current_update = updates_base + array_position;
                // updates->u[current_update] = ind[outer*5 + low];
                // updates->v[current_update] = ind[outer*5 + high];
                int current_update = updates_base + array_position;
                updates->u[current_update] = ind[low];
                updates->v[current_update] = ind[high];
                updates->dist[current_update] = total_squared_l2;
                DIST_EVAL();

                array_position++;
            }
        }

        updates->size += 2 * BLOCKSIZE; // added 10 new updates

        
        // compute the other blocks of 'outer' row which are no longer triangular__
        for(int inner = outer + 1; inner < blocks; inner++){
            for(int k = 0; k < BLOCKSIZE * BLOCKSIZE; k++){
                accs[k] = _mm256_setzero_ps();
            }

            for(int k = 0; k < BLOCKSIZE; k++){
                ind[k + BLOCKSIZE ] = vec_a->ids[inner* BLOCKSIZE + k];
            }

            for(int l = 0; l < data_dim; l+= 8){
                // load the vectors of the y axis and the x axis
                for (int k = 0; k < 2 * BLOCKSIZE; k++){
                    vector_parts[k] = _mm256_loadu_ps(&(data_values[ind[k]][l]));
                    //vector_parts[k] = _mm256_setzero_ps();
                }

                // //load the vectors of the x axis
                // for (int k = 5; k < 10; k++){
                //     vector_parts[k] = _mm256_loadu_ps(&(data_values[ind[k]][l]));
                //     // vector_parts[k] = _mm256_setzero_ps();
                // }

                
                for(int low = 0; low < BLOCKSIZE; low++){
                    for(int high = 0; high < BLOCKSIZE; high++){
                        __m256 difference = _mm256_sub_ps(vector_parts[low], vector_parts[BLOCKSIZE + high]);
                        accs[low * BLOCKSIZE + high] = _mm256_fmadd_ps(difference, difference, accs[low * BLOCKSIZE + high]);
                    }
                }    
            }

            updates_base = updates->size;
            for(int low = 0; low < BLOCKSIZE; low++){
                for(int high = 0; high < BLOCKSIZE; high++){
                    float total_squared_l2 = 0;
                    for(int k = 0; k < 8; k++){
                        total_squared_l2 += accs[low * BLOCKSIZE + high][k];
                    }
                    int current_update = updates_base + low * BLOCKSIZE + high;
                    // updates->u[current_update] = vec_a->ids[outer*5 + low];
                    // updates->v[current_update] = vec_a->ids[inner*5 + high];
                    updates->u[current_update] = ind[low];
                    updates->v[current_update] = ind[BLOCKSIZE + high];
                    updates->dist[current_update] = total_squared_l2;
                    DIST_EVAL();

                }
            }

            updates->size += BLOCKSIZE * BLOCKSIZE; // added 25 new updates
            
        }
    }

    if(vec_a_size % BLOCKSIZE != 0){
    // if(vec_a_size < 5){
        vec_t remainder;
        remainder.ids = (uint32_t*) malloc(sizeof(uint32_t) * BLOCKSIZE);
        
        int remaining_elements  = vec_a_size % BLOCKSIZE;
        // for(int i= vec_a_size / BLOCKSIZE; i < vec_a_size; i++){
        for(int i= 0; i < remaining_elements; i++){
            remainder.ids[i] = vec_a->ids[vec_a_size - remaining_elements + i];
        }

        remainder.size = remaining_elements;
        nn_brute_force(metric, data, updates, &remainder, vec_a);

        free(remainder.ids);
    }


}

void brute_force_new_unblocked(float(*metric)(float*, float*, int), dataset_t data, update_t* updates, vec_t *vec_a)
{
    int vec_a_size = vec_a->size;
    //printf("%d", vec_a_size);

    // if(vec_a_size % BLOCKSIZE != 0){
    // // if(vec_a_size < 5){
    //   nn_brute_force(metric, data, updates, vec_a, vec_a);
    //   return;
    // }

    // number of updates that will be performed
    int new_updates = vec_a_size * (vec_a_size - 1) / 2;

    float** data_values = data.values;
    int data_dim = data.dim;

    // unroll 5
    int blocks = vec_a_size / BLOCKSIZE;

    __m256 vector_parts[2* MAX_CANDIDATES]; 

    //only need 10 in the triangular part
    __m256 accs[MAX_CANDIDATES * MAX_CANDIDATES];
        
        // compute the triangular part

    // in the upper triangular part for 5x5 there are 5 (5-1) / 2 = 10 combinations
    for(int k = 0; k < MAX_CANDIDATES * MAX_CANDIDATES; k++){
        accs[k] = _mm256_setzero_ps();
    }

    uint32_t ind[MAX_CANDIDATES];

    for(int k = 0; k < vec_a_size;k++){
        ind[k] = vec_a->ids[k];
    }

    for(int l = 0; l < data_dim; l+= 8){
        for (int k = 0; k < vec_a_size; k++){
            vector_parts[k] = _mm256_loadu_ps(&(data_values[ind[k]][l]));
        }

        int array_position = 0;
        for(int low = 0; low < vec_a_size - 1; low++){
            for(int high = low + 1; high < vec_a_size; high++){
                __m256 difference = _mm256_sub_ps(vector_parts[low], vector_parts[high]);
                // accs[array_position] = _mm256_fmadd_ps(difference, difference, accs[array_position]);
                accs[low * vec_a_size + high] = _mm256_fmadd_ps(difference, difference, accs[low * vec_a_size + high]);
                array_position++;
            }
        }    
    }

    int updates_base = updates->size;
    int array_position = 0;
    for(int low = 0; low < vec_a_size - 1; low++){
        for(int high = low + 1; high < vec_a_size; high++){
            float total_squared_l2 = 0;
            for(int k = 0; k < 8; k++){
                // total_squared_l2 += accs[array_position][k];
                total_squared_l2 += accs[low * vec_a_size + high][k];
            }
            //int current_update = updates_base + array_position;
            // updates->u[current_update] = ind[outer*5 + low];
            // updates->v[current_update] = ind[outer*5 + high];
            int current_update = updates_base + array_position;
            updates->u[current_update] = ind[low];
            updates->v[current_update] = ind[high];
            updates->dist[current_update] = total_squared_l2;

            DIST_EVAL();
            array_position++;
        }
    }

    updates->size += array_position; // added 10 new updates



}


void brute_force_new_old(float(*metric)(float*, float*, int), dataset_t data, update_t* updates, vec_t *vec_a, vec_t *vec_b)
{
    // vec_a is new and vec_b is old. if vec_b == NULL do the version for new x new

    int vec_a_size = vec_a->size;
    int vec_b_size = vec_b->size;


    float** data_values = data.values;
    int data_dim = data.dim;



    uint32_t a_indices[50];
    uint32_t b_indices[50];

    for(int i = 0; i < vec_a_size; i++){
        a_indices[i] = vec_a->ids[i];

    }

    for(int i = 0; i < vec_b_size; i++){
        b_indices[i] = vec_b->ids[i];
    }

    __m256 accs[MAX_CANDIDATES * MAX_CANDIDATES];

    for(int i = 0; i < MAX_CANDIDATES * MAX_CANDIDATES; i++){
        accs[i] = _mm256_setzero_ps();
    }


    for(int l = 0; l < data_dim; l+=8){

        __m256 a_data[MAX_CANDIDATES];
        for(int a_index = 0; a_index < vec_a_size; a_index++){
            a_data[a_index] = _mm256_loadu_ps(&(data_values[a_indices[a_index]][l]));
        }

        __m256 b_data[MAX_CANDIDATES];
        for(int b_index = 0; b_index < vec_b_size; b_index++){
            b_data[b_index] = _mm256_loadu_ps(&(data_values[b_indices[b_index]][l]));
        }
        
        for(int a_index = 0; a_index < vec_a_size; a_index++)
        {
            
            for(int b_index = 0; b_index < vec_b_size; b_index++){
                __m256 difference = _mm256_sub_ps(a_data[a_index], b_data[b_index]);
                // accs[array_position] = _mm256_fmadd_ps(difference, difference, accs[array_position]);
                accs[a_index * MAX_CANDIDATES + b_index] = _mm256_fmadd_ps(difference, difference, accs[a_index * MAX_CANDIDATES + b_index]);
            }
        }
    }

    int updates_base = updates->size;
    int performed_updates = 0;
    for(int a_index  =0; a_index < vec_a_size; a_index++){
        uint32_t a_vertex = a_indices[a_index];
        for(int b_index = 0; b_index < vec_b_size; b_index++){
            // because we evaluated the distance even if the indices match
            DIST_EVAL();
            if(a_vertex == b_indices[b_index]) continue;
            performed_updates++;
            float distance = 0;
            for(int l = 0; l < data_dim; l++){
                distance += accs[a_index * MAX_CANDIDATES + b_index][l];
            }


            int current_update = updates_base + performed_updates;
            // updates->u[current_update] = vec_a->ids[outer*5 + low];
            // updates->v[current_update] = vec_a->ids[inner*5 + high];
            updates->u[current_update] = a_indices[a_index];
            updates->v[current_update] = b_indices[b_index];
            updates->dist[current_update] = distance;




        } 
    }

    updates->size += performed_updates;


}


void nn_brute_force(float(*metric)(float*, float*, int), dataset_t data, update_t* updates, vec_t *vec_a, vec_t *vec_b){
    uint32_t u1_id[8];
    uint32_t u2_id[8];
    int agg_cnt=0;

    int vec_a_size = vec_a->size;
    int vec_b_size = vec_b->size;


    for (int i = 0; i < vec_a_size; i++) {


        // if they match, can only do upper triangle due to symmetry of euclidean distance
        int start = (vec_a == vec_b) ? 0 : 0;
        for (int j = start; j < vec_b_size; j++) {
            if (vec_a->ids[i] <= vec_b->ids[j]) continue; 
            u1_id[agg_cnt] = vec_a->ids[i];
            u2_id[agg_cnt] = vec_b->ids[j];
            DIST_EVAL();
            agg_cnt++;
            if (agg_cnt < 8) continue;
            agg_cnt = 0;

            __m256 acc[8];
            for (int l=0; l<8; l++) {
                acc[l] = _mm256_setzero_ps();
            }


            for (int k=0; k < data.dim; k+=8) {

                __m256 x0[8], x1[8], x2[8];
                for (int l=0; l<8; l++) {
                    x0[l] = _mm256_loadu_ps(&(data.values[u1_id[l]][k]));
                    x1[l] = _mm256_loadu_ps(&(data.values[u2_id[l]][k]));
                    x2[l] = _mm256_sub_ps(x0[l], x1[l]);
                    acc[l] = _mm256_fmadd_ps(x2[l], x2[l], acc[l]);
                }
            }
            __m256 y01 = _mm256_hadd_ps(acc[0],acc[1]);
            __m256 y23 = _mm256_hadd_ps(acc[2],acc[3]);
            __m256 y45 = _mm256_hadd_ps(acc[4],acc[5]);
            __m256 y67 = _mm256_hadd_ps(acc[6],acc[7]);

            __m256 z0123 = _mm256_hadd_ps(y01,y23);
            __m256 z4567 = _mm256_hadd_ps(y45,y67);

            __m256 z45670123 = _mm256_blend_ps(z0123, z4567,  0b00001111);
            __m256 z45670123_flipped = _mm256_permute2f128_ps(z45670123, z45670123, 1);
            __m256 z01234567 = _mm256_blend_ps(z0123, z4567, 0b11110000);

            __m256 z = _mm256_add_ps(z01234567, z45670123_flipped);

            _mm256_storeu_si256((__m256i *)&(updates->u[updates->size]), _mm256_set_epi32(u1_id[7],u1_id[6],u1_id[5],u1_id[4],u1_id[3],u1_id[2],u1_id[1],u1_id[0]));
            _mm256_storeu_si256((__m256i *)&(updates->v[updates->size]), _mm256_set_epi32(u2_id[7],u2_id[6],u2_id[5],u2_id[4],u2_id[3],u2_id[2],u2_id[1],u2_id[0]));
            _mm256_storeu_ps(&(updates->dist[updates->size]), z);
            updates->size += 8;


        }

    }

    for (int j = 0; j < agg_cnt; j++) {
        float l = single_l2(data.values[u1_id[j]], data.values[u2_id[j]], data.dim);
            int ind = updates->size;
            updates->u[ind] = u1_id[j];
            updates->v[ind] = u2_id[j];
            updates->dist[ind] = l;
            updates->size++;
    }
    /*
       float l = metric(data.values[u1_id], data.values[u2_id], data.dim);
       updates[update_index++] = (update_t) {u1_id, u2_id, l};*/

}


            // x = _mm256_permute2f128_ps( x , x , 1)
/*
            {
                int l=5;

                float sum = ((float*)&acc[l])[0] +
                    ((float*)&acc[l])[1] +
                    ((float*)&acc[l])[2] +
                    ((float*)&acc[l])[3] +
                    ((float*)&acc[l])[4] +
                    ((float*)&acc[l])[5] +
                    ((float*)&acc[l])[6] +
                    ((float*)&acc[l])[7];
                printf("%f, %f\n", sum, z0123[0]+z0123[4]);
                printf("%f, %f\n", sum, z[5]);
                return;
                assert(sum==z[0]);
                assert(sum==((float*)&z0123)[0]+((float*)&z0123)[4]);

            /*
            {
                int l=0;

                float sum = ((float*)&acc[l])[0] +
                    ((float*)&acc[l])[1] +
                    ((float*)&acc[l])[2] +
                    ((float*)&acc[l])[3] +
                    ((float*)&acc[l])[4] +
                    ((float*)&acc[l])[5] +
                    ((float*)&acc[l])[6] +
                    ((float*)&acc[l])[7];
                assert(sum==((float*)&y01)[0]+((float*)&y01)[1]
                + ((float*)&y01)[4]+((float*)&y01)[5]);
            }*/
/*
            for (int l=0; l<8; l++) {
                float sum = ((float*)&acc[l])[0] +
                    ((float*)&acc[l])[1] +
                    ((float*)&acc[l])[2] +
                    ((float*)&acc[l])[3] +
                    ((float*)&acc[l])[4] +
                    ((float*)&acc[l])[5] +
                    ((float*)&acc[l])[6] +
                    ((float*)&acc[l])[7];
                updates[update_index++] = (update_t) {u1_id, u2_id[l], sum};
            }*/

/*            }*/
