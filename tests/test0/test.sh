#! /usr/bin/env bash

echo "---------------------"
echo "The Empty RUN test"
echo "Single run with no points"
echo "---------------------"
cat *.csv
echo "---------------------"
echo ""

echo "Expect Empty"
koviz a.csv -a

echo "Expect Empty (PDF)"
koviz a.csv -a -pdf moo.pdf
evince moo.pdf
rm moo.pdf

echo "Expect logx to be Empty"
koviz DP_logx a.csv 
echo "Expect logx to be Empty (PDF)"
~/dev/koviz/bin/koviz DP_logx a.csv -pdf moo.pdf
evince moo.pdf
rm moo.pdf

echo "Expect logy to be Empty"
koviz DP_logy a.csv 
echo "Expect logy to be Empty (PDF)"
koviz DP_logy a.csv -pdf moo.pdf
evince moo.pdf
rm moo.pdf

echo "Expect logxy to be Empty"
koviz DP_logxy a.csv 
echo "Expect logxy to be Empty (PDF)"
koviz DP_logxy a.csv -pdf moo.pdf
evince moo.pdf
rm moo.pdf
