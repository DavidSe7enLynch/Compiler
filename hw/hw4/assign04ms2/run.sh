make clean
make depend
make -j
./nearly_cc -p test/test.c
./nearly_cc -h test/test.c
./nearly_cc -h test/test.c > test/high.txt
./nearly_cc test/test.c
./nearly_cc test/test.c > test/assem.s
