#! /usr/bin/env bash

echo "---------------------"
echo "Test two point line"
echo "---------------------"
cat *.csv
echo "---------------------"
echo ""

echo "Expect linear: Two point line=(0,212F),(5,212F)"
koviz a.csv DP_linear

echo "Expect linear: Two point line=(0,212F),(5,212F) (PDF)"
koviz a.csv DP_linear -pdf moo.pdf
evince moo.pdf
rm moo.pdf

echo "Expect logxy: Single point=(5,212F)"
koviz a.csv DP_logxy

echo "Expect logxy: Single point=(5,212F) (PDF)"
koviz a.csv DP_logxy -pdf moo.pdf
evince moo.pdf
rm moo.pdf
