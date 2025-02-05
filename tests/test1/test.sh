#! /usr/bin/env bash

echo "-----------------------"
echo "Single Point (0,0) test"
echo "-----------------------"
cat *.csv
echo "---------------------"
echo ""

echo "Expect: Point=(0,0)"
koviz a.csv -a

echo "Expect: Point=(0,0) (PDF)"
koviz a.csv -a -pdf moo.pdf
evince moo.pdf
rm moo.pdf

echo "Expect: Log (0,0) makes an empty plot"
koviz a.csv DP_logxy

echo "Expect: Log (0,0) makes an empty plot (PDF)"
koviz a.csv DP_logxy -pdf moo.pdf
evince moo.pdf
rm moo.pdf
