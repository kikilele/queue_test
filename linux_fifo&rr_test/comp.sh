#/bin/sh


./test $1 99 &
usleep 1000;
./test $1 70 &
usleep 1000;

./test $1 70 &
usleep 1000;

./test $1 70 &
usleep 1000;

./test $1 50 &
usleep 1000;
./test $1 30 &
usleep 1000;
./test $1 10 &