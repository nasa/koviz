#! /usr/bin/env bash

echo "---------------------"
echo "Another 2 point test"
echo "---------------------"
cat *.csv
echo "---------------------"
echo ""

echo "Expect linear: Two point line=(0,32F),(5,212F)"
koviz a.csv DP_linear

echo "Expect linear: Two point line=(0,32F),(5,212F) (PDF)"
koviz a.csv DP_linear -pdf moo.pdf
evince moo.pdf
rm moo.pdf

echo "Expect  logxy: Single point=(5,212F)"
koviz a.csv DP_logxy

echo "Expect  logxy: Single point=(5,212F) (PDF)"
koviz a.csv DP_logxy -pdf moo.pdf
evince moo.pdf
rm moo.pdf
