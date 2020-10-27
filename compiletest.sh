cd nn_descent/
gcc knnd.c knnd_test.c vec.c bruteforce.c -lm -O3 -ffast-math -march=native
cd ..
python main.py -p nn_descent -v
