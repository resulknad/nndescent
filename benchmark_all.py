import numpy as np
import sklearn.neighbors
import os
import time
import subprocess

from benchmark import benchmark, benchmark_dim, git_clone

import argparse

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument('-r','--repetitions', help='repetitions', default=1, type=int)
    parser.add_argument('-k', help='k', default=20, type=int)
    parser.add_argument('-dim', help='dimension of space', default=128, type=int)
    parser.add_argument('-m', '--metric', help='l2', default='l2')
    parser.add_argument('-ns', '--nstart', help='logn start', default=8, type=int)
    parser.add_argument('-ne', '--nend', help='logn end', default=18, type=int)
    parser.add_argument('-nr', '--nres', help='logn resolution', default=1, type=int)

    parser.add_argument('-ds', '--dimstart', help='dim start', default=8, type=int)
    parser.add_argument('-de', '--dimend', help='dim end', default=None, type=int)
    parser.add_argument('-dst', '--dimstep', help='dim step', default=8, type=int)

    parser.add_argument('-t', '--tag', help='single tag', default=None, type=str)

    parser.add_argument('-d', '--dataset', help='audio or gaussian', default='gaussian')
    args = parser.parse_args()

    print(args)

tags = ['heap_insert_bounded', 'turbosampling', 'l2intrinsics', 'mem-align', 'blocked-distances-for-new', 'reorder-data'] if args.tag is None else [args.tag]
for t in tags:
    git_clone(t)

    if args.dimend is not None:
        print("benchmarking dimension")
        benchmark_dim(args.dataset, 2**args.nstart, 'tmp/nn_descent', args.k, 'l2', args.repetitions, args.dimstart, args.dimend, args.dimstep, t)
    else:
        print("benchmarking n")
        benchmark(args.dataset, args.dim, 'tmp/nn_descent', args.k, 'l2', args.repetitions, args.nstart, args.nend, args.nres, t)
