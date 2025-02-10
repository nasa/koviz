#! /usr/bin/env bash

echo "----------------------------------"
echo "Error plot when points outside tmt"
echo "and single point matches but not "
echo "the same"
echo "----------------------------------"
cat *.csv
echo "---------------------"
echo ""
echo "Expect error plot to be Point=(3,-0.666)"
koviz a.csv b.csv -a -pres error

echo "Expect error plot to be Point=(3,-0.666) (PDF)"
koviz a.csv b.csv -a -pres error -pdf moo.pdf
evince moo.pdf
rm moo.pdf
