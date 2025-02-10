#! /usr/bin/env bash

echo "----------------------------------"
echo "Error plot when points outside tmt"
echo "and single point that matches time"
echo "is same in both runs"
echo "----------------------------------"
cat *.csv
echo "---------------------"
echo ""
echo "Expect error plot to be Flatpoint=(3,0)"
koviz a.csv b.csv -a -pres error
echo "Expect error plot to be Flatpoint=(3,0) (PDF)"
koviz a.csv b.csv -a -pres error -pdf moo.pdf
evince moo.pdf
rm moo.pdf
