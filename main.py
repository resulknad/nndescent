import numpy as np
import sklearn.neighbors
import subprocess
from dataset import get_dataset
from dataset import GaussianDataset, AudioDataset, MnistDataset, get_dataset
from nearestneighbors import c_nearest_neighbors, py_nearest_neighbors, nearest_neighbors
from pathlib import Path
from cost import Costdata, measure_costs
import urllib.request
import argparse

parser = argparse.ArgumentParser()
parser.add_argument('-p','--path', required=True, help='path to a.out executable')
parser.add_argument('-r','--repetitions', help='repetitions', default=1, type=int)
parser.add_argument('-k', help='k', default=20, type=int)
parser.add_argument('-n', help='#points', default=None, type=int)
parser.add_argument('-clusters', help='number of clusters for ClusteredDataset', default=8, type=int)
parser.add_argument('-noshuffle', help='dont shuffle dataset (ClusteredDataset)', action='store_true', dest='noshuffle')
parser.set_defaults(noshuffle=False)

parser.add_argument('-dim', help='dimension of space', default=8, type=int)
parser.add_argument('-m', '--metric', help='l2', default='l2')
parser.add_argument('-d', '--dataset', help='audio or gaussian', default='gaussian')
parser.add_argument('-v', '--verify', help='compare recall to pynndescent', action='store_true', dest='verify')
parser.add_argument('-vc', '--verifycmp', help='compare to pynndescent', action='store_true', dest='verifycmp')
parser.set_defaults(verify=False)
parser.add_argument('-c', '--cost', help='measurements of costs (similarity evaluations)', action='store_true', dest='cost')
parser.set_defaults(cost=False)
parser.add_argument('-o', '--out', help='print stdout of c code', action='store_true', dest='out')
parser.set_defaults(out=False)
args = parser.parse_args()
print(args)


dataset = get_dataset(data_name=args.dataset, n=args.n, dim=args.dim, clusters=args.clusters, noshuffle=args.noshuffle)


nn_list, timing_data = c_nearest_neighbors(args.path, dataset, args.k, args.metric, args.repetitions, stdout=args.out)
timing_data.print()

if args.verify or args.verifycmp:
    py_nn, py_timing_data = py_nearest_neighbors(dataset, args.k, args.metric, args.repetitions)
    py_timing_data.print()

    if args.verifycmp:
        c_recall = list(map(py_nn[0].recall, nn_list))
        print("recall compared: ", c_recall)
    else:
        true_nn = nearest_neighbors(dataset, args.k, args.metric)

        py_timing_data.print()
        c_recall = list(map(true_nn.recall, nn_list))
        py_recall = list(map(true_nn.recall, py_nn))
        print("PyNNDescent: ", py_recall, ", ",args.path,": ",c_recall)
        print("Difference in mean: ", np.mean(py_recall)-np.mean(c_recall), " (lower is better)")

if args.cost:
    cost_data = measure_costs(args.path, dataset, args.k, args.metric)
    cost_data.print()
