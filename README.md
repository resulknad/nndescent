# Nearest Neighbour Descent
*see report for background: [52_report.pdf](52_report.pdf)*

This is the repository for the Advanced Systems Lab project by:
- Jonas
- Tobias
- Dan
- Samuel

The goal of the project is to implement NN-Descent which is an efficient
algorithm for approximate K-Nearest Neighbour Graph (K-NNG) construction.

The algorithm is described in the pulication "Efficient K-Nearest Neighbor
Graph Construction for Generic Similarity Measures" by Wei Dong et al.

Useful links:
 - Publication: https://www.cs.princeton.edu/cass/papers/www11.pdf
 - Project System: https://medellin.inf.ethz.ch/courses/263-2300-ETH/
 - PyNNDescent: https://github.com/lmcinnes/pynndescent

## Installation

After cloning the repository compile the C-code in the directory `nn_descent` with the following command:

```
gcc  -O3 -ffast-math -march=native -o a.out knnd.c knnd_test.c vec.c -lm
```

Additionally you will have to create a [Python virtual environment](https://docs.python.org/3/tutorial/venv.html).
To do so you can follow these steps:

1. create the environment in the chosen path (e.g ./pip-env)
    ```
    python -m venv ./pip-env
    ```
2. activate the environment `source pip-env/bin/activate`
3. install the requirements listed in requirements.txt
    `pip install -r requirements.txt`

## How-To: Benchmark
Obtain code of nn_descent of in the version you wish to benchmark. Usually this means cloning into a new directory and resetting as follows:
```
git clone ... 
cd copy_of_project
git reset --hard T0
```
Don't forget to adjust the frequency constant in the C code.

Run 
```
python benchmark.py -p ../copy_of_project/nn_descent
```

which will generate a tab separated file in directory benchmarks. Make sure to add an identifier of your architectures to files you wish to push to the repistory.

### Plotting
You may find examples of plots in eval_zenv1.ipynb
