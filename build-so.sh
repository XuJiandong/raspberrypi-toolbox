gcc -std=gnu99 -O2 -lrt -shared -o librpi-toolbox.so -fPIC *.c
cp librpi-toolbox.so lua/

