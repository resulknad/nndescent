from main import NearestNeighbors, recall, Dataset, nearest_neighbors, reference_nearest_neighbors
import unittest
import numpy as np

class TestBasicFunctionality(unittest.TestCase):

    def test_recall(self):
        nn1 = NearestNeighbors(np.array([[1,2], [3,4], [5,6], [7,8]]), 'l2')
        nn2 = NearestNeighbors(np.array([[1,9], [3,4], [5,6], [7,8]]), 'l2')
        self.assertEqual(recall(nn1, nn1), 1.0)
        self.assertEqual(recall(nn1, nn2), 0.875)

    def test_nn_l2(self):
        ds = Dataset(np.array([[0],[0.1],[1],[1.1],[2],[2.1]]))
        nn = nearest_neighbors(ds, 1, 'l2')
        self.assertTrue((nn.NN == [[1], [0],[3],[2],[5],[4]]).all())

if __name__ == '__main__':
    unittest.main()

