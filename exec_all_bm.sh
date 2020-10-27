# onle benchmark dimension 8
# python3 benchmark_all.py -ns 14 -r 1 -ds 8 -de 16 -dst 8

# dimension (showcase blocking mainly)
python3 benchmark_all.py -ns 14 -r 1 -ds 8 -de 1026 -dst 64

# low-dim perf plot
python3 benchmark_all.py -ns 7 -ne 18 -nr 2 -r 2 -dim 8

# high-dim perf plot
python3 benchmark_all.py -ns 7 -ne 18 -nr 2 -r 2 -dim 128

# higher-dim perf plot
python3 benchmark_all.py -ns 8 -ne 18 -nr 2 -r 2 -dim 256

# python3 benchmark_all.py -ns 7 -ne 16 -nr 2 -r 2 -d audio
#python3 benchmark_all.py -ns 7 -ne 16 -nr 2 -r 2 -d mnist
# python3 benchmark_all.py -ns 7 -ne 18 -nr 2 -r 2 -dim 8
