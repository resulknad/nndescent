# cost analysis

## cost measure
* the only floating point operations (add, sub, mul) are performed in the metric
* floating point comparisons


## cost occurrence
##### add, sub, mul
* metric computation

##### comparisons
* heap_insert (bubble up, max log k comparisons)
  * initialize heap list B (n*k inserts)
  * splitting up in old and new, nn_descent and sample_neighbors(n*k ???)
  * reverse heap list (unnecessary)
  * heap union (necessary ?, 2*n*k inserts)
* max_heapify (bubble down, 2 comarisons each, max 2 log k comparisons)
  * called in nn_update
* nn_update (one compare, then call max_heapify)
  * nn_descent in the most inner loop
