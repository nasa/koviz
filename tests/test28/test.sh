#! /usr/bin/env bash

echo "-----------------------------------------"
echo "-   Test Filtering Out Flatline Zeros   -"
echo "-----------------------------------------"
echo ""
echo "Expect: Popup that all curves are flat zero"
koviz a.csv a.csv -a -pres error -z
echo "Expect: Empty pdf since all filtered out"
koviz a.csv a.csv -a -pres error -z -pdf moo.pdf -platform offscreen
evince moo.pdf
rm moo.pdf
echo "Expect: Single plot x2, all else filtered out"
koviz DP_x -pres error x1.csv x2.csv -z
echo "Expect: Popup that all curves are flat/empty"
koviz -z DP_empty empty.csv
