#! /usr/bin/env bash

echo "---------------------------------------------------------"
echo "Two runs single point where points have diff x but same y"
echo "---------------------------------------------------------"
cat *.csv
echo "---------------------"
echo ""

echo "Expect linear: a=(0,1) b=(1,1)"
koviz a.csv b.csv -a
echo "Expect linear: a=(0,1) b=(1,1) (PDF)"
koviz a.csv b.csv -a -pdf moo.pdf
evince moo.pdf
rm moo.pdf

echo "Expect   logx:   a=Empty b=(1,1)"
koviz DP_logx a.csv b.csv 
echo "Expect   logx:   a=Empty b=(1,1) (PDF)"
koviz DP_logx a.csv b.csv -pdf moo.pdf
evince moo.pdf
rm moo.pdf

echo "Expect   logy:   a=(0,1) b=(1,1)"
koviz DP_logy a.csv b.csv
echo "Expect   logy:   a=(0,1) b=(1,1) (PDF)"
koviz DP_logy a.csv b.csv -pdf moo.pdf
evince moo.pdf
rm moo.pdf

echo "Expect  logxy:   a=Empty b=(1,1)"
koviz DP_logxy a.csv b.csv
echo "Expect  logxy:   a=Empty b=(1,1) (PDF)"
koviz DP_logxy a.csv b.csv -pdf moo.pdf
evince moo.pdf
rm moo.pdf
