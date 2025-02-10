#! /usr/bin/env bash

echo "---------------------------"
echo "Two runs with single points"
echo "---------------------------"
cat *.csv
echo "---------------------"
echo ""

echo "Expect linear: Point=(3,32F) foreach curve (stacked)"
koviz a.csv b.csv DP_linear

echo "Expect linear: Point=(3,32F) foreach curve (stacked) (PDF)"
koviz a.csv b.csv DP_linear -pdf moo.pdf
evince moo.pdf
rm moo.pdf

echo "Expect  logxy: Point=(3,32F) foreach curve (stacked)"
koviz a.csv b.csv DP_logxy

echo "Expect  logxy: Point=(3,32F) foreach curve (stacked) (PDF)"
koviz a.csv b.csv DP_logxy -pdf moo.pdf
evince moo.pdf
rm moo.pdf

echo "Expect  error logxy: Empty"
koviz a.csv b.csv DP_logxy -pres error

echo "Expect  error logxy: Empty (PDF)"
koviz a.csv b.csv DP_logxy -pres error -pdf moo.pdf
evince moo.pdf
rm moo.pdf
