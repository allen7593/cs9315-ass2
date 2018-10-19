rm R.*
./create R 1000 4 100
./gendata 1000 4 | ./insert R
./stats R