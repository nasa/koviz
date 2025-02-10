#! /usr/bin/env bash

echo "--------------------------"
echo "Test Alt select error plot"
echo "--------------------------"
cat *.csv
echo "---------------------"
echo ""
echo "Error plot two vars with single run with alt+click, then spacebar"
echo "Expect: flatline=-10"
~/dev/koviz/bin/koviz a.csv 
