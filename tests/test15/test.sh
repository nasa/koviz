#! /usr/bin/env bash

echo "------------------------------------------------------------"
echo "Try two single point RUNs with xs,xb,ys,yb (x values differ)"
echo "------------------------------------------------------------"
cat *.csv
echo "---------------------"
cat DP_linear
echo "---------------------"
echo ""

echo "Expect Linear: (10,10),(12,210) 210=100*2+10"
~/dev/koviz/bin/koviz DP_linear a.csv b.csv
echo "Expect Linear: (10,10),(12,210) 210=100*2+10 (PDF)"
~/dev/koviz/bin/koviz DP_linear a.csv b.csv -pdf moo.pdf
evince moo.pdf
rm moo.pdf

echo "Expect Logx: (10,10),(12,210) 210=100*2+10"
~/dev/koviz/bin/koviz DP_logx a.csv b.csv 
echo "Expect Logx: (10,10),(12,210) 210=100*2+10 (PDF)"
~/dev/koviz/bin/koviz DP_logx a.csv b.csv -pdf moo.pdf
evince moo.pdf
rm moo.pdf

echo "Expect Logy: (10,10),(12,210) 210=100*2+10"
~/dev/koviz/bin/koviz DP_logy a.csv b.csv 
echo "Expect Logy: (10,10),(12,210) 210=100*2+10 (PDF)"
~/dev/koviz/bin/koviz DP_logy a.csv b.csv -pdf moo.pdf
evince moo.pdf
rm moo.pdf

echo "Expect Logxy: (10,10),(12,210) 210=100*2+10"
~/dev/koviz/bin/koviz DP_logxy a.csv b.csv
echo "Expect Logxy: (10,10),(12,210) 210=100*2+10 (PDF)"
~/dev/koviz/bin/koviz DP_logxy a.csv b.csv -pdf moo.pdf
evince moo.pdf
rm moo.pdf
