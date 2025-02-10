#! /usr/bin/env bash

echo "-------------------------"
echo "Test DP_logxy when x is 0"
echo "-------------------------"
cat *.csv
echo "---------------------"
echo ""

echo "Expect: empty plot"
~/dev/koviz/bin/koviz a.csv DP_logxy
echo "Expect: empty plot (PDF)"
~/dev/koviz/bin/koviz a.csv DP_logxy -pdf moo.pdf
evince moo.pdf
rm moo.pdf
