rm R.*
./create R 1000 4 1000
./gendata 1000 4 | ./insert R
./stats R