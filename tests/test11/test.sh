#! /usr/bin/env bash

echo "-----------------------"
echo "Two runs with no points"
echo "-----------------------"
cat *.csv
echo "---------------------"
echo ""

echo "Expect Empty"
~/dev/koviz/bin/koviz a.csv b.csv -a
echo "Expect Empty (PDF)"
~/dev/koviz/bin/koviz a.csv b.csv -a -pdf moo.pdf
evince moo.pdf
rm moo.pdf

echo "Expect logx Empty"
~/dev/koviz/bin/koviz DP_logx a.csv b.csv 
echo "Expect logx Empty (PDF)"
~/dev/koviz/bin/koviz DP_logx a.csv b.csv -pdf moo.pdf
evince moo.pdf
rm moo.pdf

echo "Expect logy Empty"
~/dev/koviz/bin/koviz DP_logy a.csv b.csv
echo "Expect logy Empty (PDF)"
~/dev/koviz/bin/koviz DP_logy a.csv b.csv -pdf moo.pdf
evince moo.pdf
rm moo.pdf

echo "Expect logxy Empty"
~/dev/koviz/bin/koviz DP_logxy a.csv b.csv
echo "Expect logxy Empty (PDF)"
~/dev/koviz/bin/koviz DP_logxy a.csv b.csv -pdf moo.pdf
evince moo.pdf
rm moo.pdf
