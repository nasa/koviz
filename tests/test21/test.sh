#! /usr/bin/env bash

echo "------------------------------------------------"
echo "RUN_a and RUN_b split x & y into different files"
echo "Time units differ in all files"
echo "In RUN_b, time units are set in map file"
echo "I thought about putting units in timeName, "
echo "but it was ugly underneath, so the map file"
echo "is the way to set units for time if needed"
echo "------------------------------------------------"
echo "Koviz session"
cat session_test
echo "---------------------"
echo "RUN_a/ax.csv"
cat RUN_a/ax.csv
echo ""
echo "RUN_a/ay.csv"
cat RUN_a/ay.csv
echo ""
echo "RUN_b/bx.csv"
cat RUN_b/bx.csv
echo ""
echo "RUN_b/by.csv"
cat RUN_b/by.csv
echo ""
echo "---------------------"
echo ""
echo "Expect: one diamond inside another diamond"
echo "Outside diamond: (10,0),(0,10),(-10,0),(0,-10),(10,0)"
echo "Inside diamond:  (5,0),(0,5),(-5,0),(0,-5),(5,0)"
koviz -session session_test
