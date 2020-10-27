import numpy as np
import sklearn.metrics.pairwise
from pathlib import Path
import urllib.request
import pandas as pd

def get_dataset(data_name, n, dim, clusters=None, noshuffle=False):
    if ((dim % 8 != 0)):
        print("Choose dimensionality which is divisible by 8 (restriction)")
        exit(1)

    if data_name == 'gaussian':
        # n datapoints sampled from each of dim gaussians centered around canonical basis vector with dimension dim
        n = 1000 if n is None else n
        return GaussianDataset(dimension=dim, variance=2.0, n=n)
    elif data_name == 'clustered':
        # n datapoints sampled from each of dim gaussians centered around canonical basis vector with dimension dim
        n = 1000 if n is None else n
        return ClusteredDataset(dimension=dim, n=n, clusters=clusters, noshuffle=noshuffle)
    elif data_name == 'singlegaussian':
        n = 1000 if n is None else n
        return SingleGaussianDataset(dimension=dim, variance=2.0, n=n)
    elif data_name == 'audio':
        # Audio dataset as described in the NN-Descent publication
        #  54,387 points (192 dimensional)
        my_file = Path("audio.data")
        if not my_file.is_file():
            print("audio.data not here, downloading...")
            urllib.request.urlretrieve ("http://kluser.ch/audio.data", "audio.data")
        return AudioDataset(n)
    elif data_name == 'mnist' or data_name == 'digits':
        # MNIST dataset of 70k handwritten digits (784 dimensional)

        mnist_filenames = ["mnist_train.csv","mnist_test.csv"]

        for csv_file in mnist_filenames:
            if not Path(csv_file).is_file():
                print("downloading " + csv_file)
                urllib.request.urlretrieve("https://pjreddie.com/media/files/" + csv_file, csv_file)
        return MnistDataset(n)

    elif data_name == 'pca_mnist':
        return MnistSortedDataset(umap=False)

    elif data_name == 'umap_mnist':
        return MnistSortedDataset(umap=True)
    else:
        print("dataset not supported")
        exit(1)


class Dataset:
    def __init__(self,X):
        self.X = X
        self.N = X.shape[0]
        self.D = X.shape[1]

    def save(self, filename):
        # save dataset to space separated values file
        np.savetxt(filename, self.X)

class SingleGaussianDataset(Dataset):
    def __init__(self, dimension, variance, n):

        cov = variance * np.identity(dimension)
        X = []
        mean = np.zeros(dimension)
        X = np.random.multivariate_normal(mean, cov, n)
        np.random.shuffle(X)
        Dataset.__init__(self, X)

class ClusteredDataset(Dataset):
    def __init__(self, dimension, clusters, n, noshuffle):
        print("Dimension",dimension, ", clusters:", clusters, ", n:",n, ", shuffle: ",not noshuffle)
        # n=total points, equally distributed on clusters. each point has dimension dimensions

        def min_dist(mean_matrix):
            curr_min = np.inf
            c = sklearn.metrics.pairwise_distances(mean_matrix, mean_matrix)
            np.fill_diagonal(c, np.inf)
            print(c.min())
            return c.min()

        # find separate cluster means
        separate_cluster_means = False
        means = []
        while not separate_cluster_means:
            print("generating means")
            means = np.array([np.random.randint(0,1000000)*(np.random.rand(dimension)-np.repeat(0.5, dimension)) for i in range(clusters)])
            if min_dist(means)>1000:
                separate_cluster_means = True

        self.means = means
        cov = 1 * np.identity(dimension)
        X = []
        for i in range(clusters):
            mean = np.zeros(dimension)
            X.append(np.random.multivariate_normal(means[i], cov, n//clusters))
        X = np.vstack(X)
        if not noshuffle:
            np.random.shuffle(X)
            print("shuffled")
        print("done with dataset")
        Dataset.__init__(self, X)


class GaussianDataset(Dataset):
    def __init__(self, dimension, variance, n):
        # generate n//dimension points from each of dimension many sperical gaussians
        # the gaussians are each centered around a canonical basisvector
        # n//dimension*dimension many points in total

        cov = variance * np.identity(dimension)
        X = []
        for i in range(dimension):
            mean = np.zeros(dimension)
            mean[i] = 1.0
            X.append(np.random.multivariate_normal(mean, cov, n//dimension))
        X = np.vstack(X)
        np.random.shuffle(X)
        Dataset.__init__(self, X)

class AudioDataset(Dataset):

    # Audio dataset as described in the publicaiton. 54,387 points (192 dimensional)
    def __init__(self, n=0):
        path = 'audio.data'
        # read data as specified here
        # http://lshkit.sourceforge.net/dc/d46/matrix_8h.html
        with open(path, "rb") as f:
            def read_uint(uint_bytes):
                return int.from_bytes(uint_bytes, signed=False, byteorder='little')

            elem_size = read_uint(f.read(4))
            size = read_uint(f.read(4))
            dim_elem_size = read_uint(f.read(4))

            # reads all 4-byte floats into flat array of dimension size*dim_elem_size
            X = np.frombuffer(np.array(f.read(4*size*dim_elem_size)), dtype=np.float32);

            X = X.reshape((size,dim_elem_size))
            np.random.shuffle(X)

            # take not the whole dataset
            if (n!=0):
                X = X[:n,:]


            self.X = X

#            X = np.zeros((size,dim_elem_size))
#
#            for i in range(size):
#                for j in range(dim_elem_size):
#                    X[i,j] = np.array(f.read(4)).view(dtype=np.float32)



        Dataset.__init__(self, X)

class MnistDataset(Dataset):

    # Default MNIST Dataset 70k handwritten digits (784 dimensional)

    def __init__(self, n):
        train = 'mnist_train.csv' # 60k
        test = 'mnist_test.csv' # 10k

        train_df = pd.read_csv(train, header=None)
        test_df = pd.read_csv(test, header=None)

        complete_df = pd.concat([train_df, test_df], axis = 0)

        X = complete_df.iloc[:,1:].to_numpy(dtype='float32')
        np.random.shuffle(X)
        if (n!=0):
            X = X[:n,:]

        self.X = X
        Dataset.__init__(self, X)

        #self.N = complete_df.shape[0] # 70,000
        #self.D = complete_df.shape[1] - 1 # 784


class MnistSortedDataset(Dataset):

    # MNIST dataset sorted according to a 1d umap

    def __init__(self, umap=True):
        if umap:
            df = pd.read_csv('mnist_sort_umap.csv', header=None)
        else:
            df = pd.read_csv('mnist_sort_pca.csv', header=None)



        self.X = df.to_numpy(dtype='float32')
        self.N = df.shape[0] # 70,000
        self.D = df.shape[1] # 784



