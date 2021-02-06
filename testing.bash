./o++ -debug ./tests/test.opp
elfdebug out.o
readelf -a -t out.o >> out
objdump -d -M intel out.o
