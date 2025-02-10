#! /usr/bin/env bash

echo "-----------------------------------"
echo "Another two RUNs with single points"
echo "-----------------------------------"
cat *.csv
echo "---------------------"
echo ""

echo "Expect linear: a.csv=(3,32F) b.csv=(3,212F)"
koviz a.csv b.csv DP_linear

echo "Expect linear: a.csv=(3,32F) b.csv=(3,212F) (PDF)"
koviz a.csv b.csv DP_linear -pdf moo.pdf
evince moo.pdf
rm moo.pdf

echo "Expect  logxy: a.csv=(3,32F) b.csv=(3,212F)"
koviz a.csv b.csv DP_logxy

echo "Expect  logxy: a.csv=(3,32F) b.csv=(3,212F) (PDF)"
koviz a.csv b.csv DP_logxy -pdf moo.pdf
evince moo.pdf
rm moo.pdf

echo "Expect  error logxy: (3,180F) note its +180 since logscale flips sign"
koviz a.csv b.csv DP_logxy -pres error

echo "Expect  error logxy: (3,180F) note its +180 since logscale flips sign (PDF)"
koviz a.csv b.csv DP_logxy -pres error -pdf moo.pdf
evince moo.pdf
rm moo.pdf
