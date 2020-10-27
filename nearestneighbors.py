import numpy as np
import subprocess
import sklearn
import os
import time
from timing import parse_output, Timingdata
from numba import set_num_threads
set_num_threads(1)
from pynndescent import NNDescent

class NearestNeighbors:
    # initialization with either a file
    # or 2d numpy array
    def __init__(self, NN=None, metric='l2', filename=None, stdout=""):
        if filename is None:
            self.NN = NN
        else:
            self.load(filename)

        self.N = self.NN.shape[0]
        self.K = self.NN.shape[1]

        self.metric = metric

        self.stdout = stdout

    # calculates recall (assuming one of the datasets
    # represents the ground truth)
    def recall(self, other):
        if self.NN.shape != other.NN.shape:
            raise ValueError('dimensions of nearest neighbors dont match')

        if self.metric != other.metric:
            raise ValueError('metrics dont match')

        common_neighbors =list(map(lambda row1,row2:
                                   len(np.intersect1d(row1,row2)),
                                   self.NN,
                                   other.NN))

        return float(np.sum(common_neighbors))/(self.K*self.N)

    def load(self, filename):
        self.NN = np.loadtxt(filename)

# performs brute-force KNN on dataset
def nearest_neighbors(dataset, K, metric):
    if metric == 'l2':
        scikit_metric = 'minkowski'
        scikit_p = 2
    else:
        raise ValueError(metric + ' not implemented')

    # calculates K nearest neighbors for given dataset
    nbrs = sklearn.neighbors.NearestNeighbors(n_neighbors=K, algorithm='brute', metric=scikit_metric, p=scikit_p).fit(dataset.X)

    # gives back k nearest neighbors of training data
    # points can't be their own nn
    nbrs_indices = nbrs.kneighbors(return_distance=False)
    return NearestNeighbors(nbrs_indices, metric)

# performs reference knndescent on dataset
# returns nearestneighbors and timing
def c_nearest_neighbors(directory, dataset, K, metric, repetition, stdout=False, gprof_compile=False):
    # calls reference implementation for NN
    files = ['knnd.c', 'knnd_test.c', 'vec.c']

    # was only added in recent commits, should remain backwards compatible
    if os.path.isfile(os.path.join(directory, 'bruteforce.c')):
        files.append('bruteforce.c')

    flags = ['-lm','-O3','-ffast-math','-march=native']
    if gprof_compile:
        process = subprocess.run(['gcc'] + files + flags + ['-pg', '-DINSTR=true'], check=True, stdout=subprocess.PIPE, universal_newlines=True, cwd=directory)
    else:
        process = subprocess.run(['gcc'] + files + flags + ['-flto', '-DCALIBRATE'], check=True, stdout=subprocess.PIPE, universal_newlines=True, cwd=directory)

    if metric != 'l2':
        raise ValueError(metric + ' not implemented')
    dataset.save('data')

    path = os.path.join(directory, "a.out")

    cycles = np.zeros(repetition)
    runtime = np.zeros(repetition)
    nn_list = []

    for i in range(repetition):
        process = subprocess.run([path,'data','output', str(dataset.N), str(dataset.D), str(K)], check=True, stdout=subprocess.PIPE, universal_newlines=True)
        if stdout: print(process.stdout)

        c,t  = parse_output(process.stdout.splitlines())
        nn_data = NearestNeighbors(filename='output', stdout=process.stdout)

        cycles[i] = c
        runtime[i] = t
        nn_list.append(nn_data)

    return nn_list, Timingdata(cycles,runtime, directory)



def py_nearest_neighbors(dataset, K, metric, repetition):
    # (try) to enforce pynndescent using only a single thread:

    # a point is his own NN
    # thats why we query K+1 and remove afterwards

    runtime = np.zeros(repetition+1)
    nn_list = []
    for i in range(repetition+1):
        start = time.perf_counter()
        index = NNDescent(dataset.X,
                          n_neighbors=(K+1),
#                          verbose=True,
                          tree_init=False, # some fancy init
                          n_jobs=1)

        elapsed = time.perf_counter()-start
        runtime[i] = elapsed

        nn_arr = index._neighbor_graph[0]
        assert((nn_arr[:,0] == np.array(range(dataset.N))).all())
        nn_list.append(NearestNeighbors(nn_arr[:,1:], metric))

    # skip first repetition, since JIT does a lot of work then...
    return nn_list[1:], Timingdata(None, runtime[1:], "pynndescent")
