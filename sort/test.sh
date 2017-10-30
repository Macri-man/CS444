#!/bin/bash

make clean
make

# Different number of threads, 100 files, length 100
echo "Varying threads, 100 files, 100 lines"
./tg.sh 0 100 100
time ./a.out thread/ 1
rm thread/*.sorted

./tg.sh 0 100 100
time ./a.out thread/ 2
rm thread/*.sorted

./tg.sh 0 100 100
time ./a.out thread/ 4
rm thread/*.sorted

./tg.sh 0 100 100
time ./a.out thread/ 8
rm thread/*.sorted

./tg.sh 0 100 100
time ./a.out thread/ 16
rm thread/*.sorted

# Different number of lines, 10 files, 4 threads
echo "Lines 100, 200, 400, 800, 1600, 100 files, 4 threads"
./tg.sh 0 100 100
time ./a.out thread/ 4
rm thread/*.sorted

./tg.sh 0 100 200
time ./a.out thread/ 4
rm thread/*.sorted

./tg.sh 0 100 400
time ./a.out thread/ 4
rm thread/*.sorted

./tg.sh 0 100 800
time ./a.out thread/ 4
rm thread/*.sorted

./tg.sh 0 100 1600
time ./a.out thread/ 4
rm thread/*.sorted

# Different number of files, 4 threads, lenth 10
echo "Files 400, 800, 1600, 3200, 6400, 4 threads, length 10"
./tg.sh 0 400 10
time ./a.out thread/ 4
rm thread/*.sorted

./tg.sh 0 800 10
time ./a.out thread/ 4
rm thread/*.sorted

./tg.sh 0 1600 10
time ./a.out thread/ 4
rm thread/*.sorted

./tg.sh 0 3200 10
time ./a.out thread/ 4
rm thread/*.sorted

./tg.sh 0 6400 10
time ./a.out thread/ 4
rm thread/*.sorted