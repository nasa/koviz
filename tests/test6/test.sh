#! /usr/bin/env bash

echo "----------------------------------------------"
echo "Two two-point flat line runs that are the same"
echo "----------------------------------------------"
cat *.csv
echo "---------------------"
echo ""
echo ""

echo "Expect linear: {(0,212F),(5,212F)}"
koviz a.csv b.csv DP_linear

echo "Expect linear: {(0,212F),(5,212F)} (PDF)"
koviz a.csv b.csv DP_linear -pdf moo.pdf
evince moo.pdf
rm moo.pdf

echo "Expect  logxy: (5,212F)"
koviz a.csv b.csv DP_logxy

echo "Expect  logxy: (5,212F) (PDF)"
koviz a.csv b.csv DP_logxy -pdf moo.pdf
evince moo.pdf
rm moo.pdf

echo "Expect  error logxy: Empty"
koviz a.csv b.csv DP_logxy -pres error

echo "Expect  error logxy: Empty (PDF)"
koviz a.csv b.csv DP_logxy -pres error -pdf moo.pdf
evince moo.pdf
rm moo.pdf
