import numpy as np
import sklearn.neighbors
import os
import time
import subprocess
from dataset import get_dataset
from nearestneighbors import c_nearest_neighbors, py_nearest_neighbors, nearest_neighbors

from cost import Costdata, measure_costs
import argparse
def git_clone(tag):
# delete temp directory
    subprocess.run(['rm', '-rf','tmp'])

# clone repo
    subprocess.run(['git', 'clone', 'git@gitlab.inf.ethz.ch:COURSE-ASL2020/team052.git', 'tmp'])

# set tag
    subprocess.run(['git', 'checkout', tag], cwd='tmp')

def save_data(fname, n, simi_evals, runtime_s, runtime_cycles, cycles_std, flops):
    X = np.array([n, simi_evals, runtime_s, runtime_cycles, cycles_std, flops]).transpose()
    np.savetxt(os.path.join('benchmarking', fname), X)

def benchmark(dataset_name, dim, path, k, metric, repetitions, n_start, n_end, n_res, prefix):
    inputs = []
    cycles = []
    cycles_std = []
    runtimes = []
    sim_evals = []

    for n in np.logspace(n_start, n_end, num=n_res*(n_end-n_start+1), dtype=int, base=2):

        dataset = get_dataset(dataset_name, n,dim)
        inputs.append(dataset.N)

        nn_list, timing_data = c_nearest_neighbors(path, dataset, k, metric, repetitions)
        # append median or avg?
        cycles.append(timing_data.median_cycle)
        cycles_std.append(timing_data.std_cycle)
        runtimes.append(timing_data.median_runtime)

        cost_data = measure_costs(path, dataset, k, metric)
        sim_evals.append(cost_data.metric_calls)
    print(sim_evals)

# for L2 norm:
# d operations for a[i]-b[i]
# d operations for squaring each component
# d-1 operations for summing up the squares
# so 3d flops per sim evaluation

    flops = np.array(sim_evals)*dataset.D*(3-1)

    save_data('{}_{}_dim{}_logn{}to{}_k{}'.format(prefix, dataset_name,dim, n_start, n_end, k), inputs, sim_evals, runtimes, cycles, cycles_std, flops)

def benchmark_dim(dataset_name, n, path, k, metric, repetitions, dim_start, dim_end, dim_step, prefix):
    inputs = []
    cycles = []
    cycles_std = []
    runtimes = []
    sim_evals = []
    flops = []

    for dim in np.arange(dim_start, dim_end, dim_step):
        inputs.append(dim)
        dataset = get_dataset(dataset_name, n,dim)

        nn_list, timing_data = c_nearest_neighbors(path, dataset, k, metric, repetitions)
        # append median or avg?
        cycles.append(timing_data.median_cycle)
        cycles_std.append(timing_data.std_cycle)
        runtimes.append(timing_data.median_runtime)

        cost_data = measure_costs(path, dataset, k, metric)
        sim_evals.append(cost_data.metric_calls)
        flops.append(cost_data.metric_calls*dataset.D*(3-1))
        print("Dim: ",dim,", flops:",flops[len(flops)-1]/cycles[len(cycles)-1])

# for L2 norm:
# d operations for a[i]-b[i]
# d operations for squaring each component
# d-1 operations for summing up the squares
# so 3d flops per sim evaluation

    save_data('{}_{}_n{}_dim{}to{}_k{}'.format(prefix, dataset_name, n, dim_start, dim_end, k), inputs, sim_evals, runtimes, cycles, cycles_std, flops)


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument('-p','--path', required=True, help='path to a.out executable')
    parser.add_argument('-r','--repetitions', help='repetitions', default=1, type=int)
    parser.add_argument('-k', help='k', default=20, type=int)
    parser.add_argument('-dim', help='dimension of space', default=100, type=int)
    parser.add_argument('-m', '--metric', help='l2', default='l2')
    parser.add_argument('-d', '--dataset', help='audio or gaussian', default='gaussian')
    parser.add_argument('-ns', '--nstart', help='logn start', default=8, type=int)
    parser.add_argument('-ne', '--nend', help='logn end', default=18, type=int)
    parser.add_argument('-nr', '--nres', help='logn resolution', default=1, type=int)
    args = parser.parse_args()

    START = 10
    END = 10
    RESOLUTION = 1
    print(args)

    benchmark(args.dataset, args.dim, args.path, args.k, args.metric, args.repetitions, args.nstart, args.nend, args.nres, '')
