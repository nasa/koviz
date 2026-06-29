#! /usr/bin/env bash

echo "-----------------------"
echo "Two runs with no points"
echo "-----------------------"
cat *.csv
echo "---------------------"
echo ""

echo "Expect Empty"
koviz a.csv b.csv -a
echo "Expect Empty (PDF)"
koviz a.csv b.csv -a -pdf moo.pdf
evince moo.pdf
rm moo.pdf

echo "Expect logx Empty"
koviz DP_logx a.csv b.csv 
echo "Expect logx Empty (PDF)"
koviz DP_logx a.csv b.csv -pdf moo.pdf
evince moo.pdf
rm moo.pdf

echo "Expect logy Empty"
koviz DP_logy a.csv b.csv
echo "Expect logy Empty (PDF)"
koviz DP_logy a.csv b.csv -pdf moo.pdf
evince moo.pdf
rm moo.pdf

echo "Expect logxy Empty"
koviz DP_logxy a.csv b.csv
echo "Expect logxy Empty (PDF)"
koviz DP_logxy a.csv b.csv -pdf moo.pdf
evince moo.pdf
rm moo.pdf
