make clean
make depend
make -j
./nearly_cc -p test/test.c
./nearly_cc -a test/test.c
