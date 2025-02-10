#! /usr/bin/env bash

echo "----------------------------------------------------"
echo "Test single run with all values non-zero except one,"
echo "and then do y log scale to see if culling to a single"
echo "y-val in logscale works"
echo "----------------------------------------------------"
cat *.csv
echo "---------------------"
echo ""
echo "Expect Point=(1,1)"
koviz a.csv DP_test 

echo "Expect Point=(1,1) (PDF)"
koviz a.csv DP_test -pdf moo.pdf
evince moo.pdf
rm moo.pdf
