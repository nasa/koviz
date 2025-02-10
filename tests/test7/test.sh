#! /usr/bin/env bash

echo "------------------------------"
echo "Another two two-point run test"
echo "------------------------------"
cat *.csv
echo "---------------------"
echo ""

echo "Expect linear: a=(0,32),(5,212) b=(0,32),(5,77)"
koviz a.csv b.csv DP_linear
echo "Expect linear: a=(0,32),(5,212) b=(0,32),(5,77) (PDF)"
koviz a.csv b.csv DP_linear -pdf moo.pdf
evince moo.pdf
rm moo.pdf

echo "Expect linerr: e=(0,0),(5,135)"
koviz a.csv b.csv DP_linear -pres error
echo "Expect linerr: e=(0,0),(5,135) (PDF)"
koviz a.csv b.csv DP_linear -pres error -pdf moo.pdf
evince moo.pdf
rm moo.pdf

echo "Expect log: a=(5,212) b=(5,77)"
koviz a.csv b.csv DP_logxy
echo "Expect log: a=(5,212) b=(5,77) (PDF)"
koviz a.csv b.csv DP_logxy -pdf moo.pdf
evince moo.pdf
rm moo.pdf

echo "Expect logerr: e=(5,135)"
koviz a.csv b.csv DP_logxy -pres error
echo "Expect logerr: e=(5,135) (PDF)"
koviz a.csv b.csv DP_logxy -pres error -pdf moo.pdf
evince moo.pdf
rm moo.pdf
