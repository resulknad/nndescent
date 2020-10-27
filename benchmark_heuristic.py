import numpy as np
from io import StringIO
import sklearn.neighbors
from sklearn.neighbors import NearestNeighbors
import os
import time
import subprocess

from dataset import get_dataset
from nearestneighbors import c_nearest_neighbors
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

# script that benchmarks the permutation of the greedy heuristic
# clones modified nn_descent into tmp which prints permutation and exits

result = []
n=2**14
dim=8
clusters=8
dataset = get_dataset(data_name='clustered', n=n, dim=dim, clusters=clusters, noshuffle=False)
git_clone('reorder-print-perm')
nn_list, no_reorder = c_nearest_neighbors('tmp/nn_descent', dataset, args.k, args.metric, args.repetitions)
s= nn_list[0].stdout

start = s.find("fwd_permutation\n") + len("fwd_permutation\n")
end = s.find("fwd_permutation_end")
substring = s[start:end]
fwd_perm = np.loadtxt(StringIO(substring), dtype=int)

X = dataset.X
X_ = np.zeros(X.shape)

for i in range(len(fwd_perm)):
    X_[fwd_perm[i]] = X[i]

nbrs = NearestNeighbors(n_neighbors=1).fit(dataset.means)
distances, indices = nbrs.kneighbors(X_)

window = 1000
n = len(fwd_perm)
freqs = []
clusters =  sorted(list(set(indices.flatten())))

for cluster in clusters:
    occurences = list(map(lambda i: np.sum(indices[i-window:i+window]==cluster), range(window, n-window)))
    freq = np.true_divide(occurences, 2*window)
    freqs.append(freq)
print("header: ", clusters)
np.savetxt('clust_freq.txt', np.array(freqs).transpose())
