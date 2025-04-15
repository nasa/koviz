#!/bin/sh

./clean.sh

# For C and test
cc -c koviz.c
ar cr libkoviz.a koviz.o
cc -o testme main.c libkoviz.a -lrt

# For TS21
#cc -I/users/cxtfcm/TRICK_DB/15.1.0-patch4/trick/trick_source -c koviz.c
#ar cr libkoviz.a koviz.o

# For C++
# g++ -I/home/kvetter/dev/trick/include -c koviz.c
# ar cr libkoviz.a koviz.o
