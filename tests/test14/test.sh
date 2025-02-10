#! /usr/bin/env bash

echo "---------------------------------"
echo "Try single point with xs,xb,ys,yb"
echo "---------------------------------"
cat a.csv
echo "---------------------"
echo ""

echo "Expect Linear: (12,74F) 12=1*2+10 74=32*2+10"
~/dev/koviz/bin/koviz DP_linear a.csv
echo "Expect Linear: (12,74F) 12=1*2+10 74=32*2+10 (PDF)"
~/dev/koviz/bin/koviz DP_linear a.csv -pdf moo.pdf
evince moo.pdf
rm moo.pdf

echo "Expect Logx: (12,74F) 12=1*2+10 74=32*2+10"
~/dev/koviz/bin/koviz DP_logx a.csv
echo "Expect Logx: (12,74F) 12=1*2+10 74=32*2+10 (PDF)"
~/dev/koviz/bin/koviz DP_logx a.csv -pdf moo.pdf
evince moo.pdf
rm moo.pdf

echo "Expect Logy: (12,74F) 12=1*2+10 74=32*2+10"
~/dev/koviz/bin/koviz DP_logy a.csv
echo "Expect Logy: (12,74F) 12=1*2+10 74=32*2+10 (PDF)"
~/dev/koviz/bin/koviz DP_logy a.csv -pdf moo.pdf
evince moo.pdf
rm moo.pdf

echo "Expect Logxy: (12,74F) 12=1*2+10 74=32*2+10"
~/dev/koviz/bin/koviz DP_logxy a.csv
echo "Expect Logxy: (12,74F) 12=1*2+10 74=32*2+10 (PDF)"
~/dev/koviz/bin/koviz DP_logxy a.csv -pdf moo.pdf
evince moo.pdf
rm moo.pdf
