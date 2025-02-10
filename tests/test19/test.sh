#! /usr/bin/env bash

echo "--------------------------------------"
echo "Test tmt to match points in error plot"
echo "--------------------------------------"
cat *.csv
echo "---------------------"
echo ""

echo "Expect (1.01,136),(2.95,90),(5.1,360),(8.0,0)"
/home/kvetter/dev/koviz/bin/koviz a.csv b.csv DP_test -pres error -tmt 0.1
