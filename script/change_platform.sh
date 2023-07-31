#! /bin/sh

make clean
cd lib
rm -f libdouble_link_list.a
rm -f libhiredis.a

cd double_link_list
make clean
make
cp libdouble_linked_list.a ../

cd ..
cd hiredis-1.1.0
make clean
make
cp libhiredis.a ../
cd ../..

make
