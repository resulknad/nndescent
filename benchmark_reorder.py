import numpy as np
import sklearn.neighbors
import re
import os
import time
import subprocess

from dataset import get_dataset
from nearestneighbors import c_nearest_neighbors
from benchmark import benchmark, benchmark_dim, git_clone
import argparse

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument('-d', '--dataset', help='clustered or mnist', default='clustered')
    args = parser.parse_args()

    print(args)
def extract_iteration_timing(s):
    pattern = 'Iteration \d: ([^ ]*) seconds'
    matches = re.findall(pattern, s)
    return matches


def speedup(n, clusters, dim):
    if args.dataset == 'clustered':
        dataset = get_dataset(data_name='clustered', n=n, dim=dim, clusters=clusters, noshuffle=False)
        print("Clustered Dataset")
    else:
        dataset = get_dataset(data_name='mnist', n=None, dim=8)
        print("MNIST Dataset")

    git_clone('blocked-distances-for-new-print-iterations')
    nn_list, no_reorder = c_nearest_neighbors('tmp/nn_descent', dataset, 20, 'l2', 2)
    baseline = extract_iteration_timing(nn_list[0].stdout)

    git_clone('reorder-data')
    nn_list, reorder = c_nearest_neighbors('tmp/nn_descent', dataset, 20, 'l2', 2)
    reordered = extract_iteration_timing(nn_list[0].stdout)

    return baseline, reordered, no_reorder.median_cycle/ reorder.median_cycle
result = []
baseline, reordered, speedup = speedup(n=2**17, clusters=16, dim=8)
print("speedup: ", speedup)
print("baseline iterations:  ", baseline)
print("reordered iterations: ", reordered)
