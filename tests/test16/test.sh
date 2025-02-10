#! /usr/bin/env bash

echo "------------------------------------------------------------------------"
echo "Try two single point RUNs with xs,xb,ys,yb (x values same, yvals differ)"
echo "x = 0 = -5*2+10 434=212*2+10 (F native in b.csv)"
echo "------------------------------------------------------------------------"
cat *.csv
echo "---------------------"
cat DP_linear
echo "---------------------"
echo ""

echo "Expect Linear: (0,74),(0,434)"
~/dev/koviz/bin/koviz DP_linear a.csv b.csv
echo "Linear Linear: (0,74),(0,434) (PDF)"
~/dev/koviz/bin/koviz DP_linear a.csv b.csv -pdf moo.pdf
evince moo.pdf
rm moo.pdf

echo "Expect Logx: Empty"
~/dev/koviz/bin/koviz DP_logx a.csv b.csv 
echo "Expect Logx: Empty (PDF)"
~/dev/koviz/bin/koviz DP_logx a.csv b.csv -pdf moo.pdf
evince moo.pdf
rm moo.pdf

echo "Expect Logy: (0,74),(0,434)"
~/dev/koviz/bin/koviz DP_logy a.csv b.csv 
echo "Expect Logy: (0,74),(0,434) (PDF)"
~/dev/koviz/bin/koviz DP_logy a.csv b.csv -pdf moo.pdf
evince moo.pdf
rm moo.pdf

echo "Expect Logxy: Empty"
~/dev/koviz/bin/koviz DP_logxy a.csv b.csv
echo "Expect Logxy: Empty (PDF)"
~/dev/koviz/bin/koviz DP_logxy a.csv b.csv -pdf moo.pdf
evince moo.pdf
rm moo.pdf

echo "Expect Error plot Flatpoint=(0,-360) -360=74-434"
~/dev/koviz/bin/koviz DP_linear a.csv b.csv -pres error
echo "Expect Error plot Flatpoint=(0,-360) -360=74-434 (PDF)"
~/dev/koviz/bin/koviz DP_linear a.csv b.csv -pres error -pdf moo.pdf
evince moo.pdf
rm moo.pdf

echo "Expect Error plot Logx: Empty"
~/dev/koviz/bin/koviz DP_logx a.csv b.csv -pres error
echo "Expect Error plot Logx: Empty (PDF)"
~/dev/koviz/bin/koviz DP_logx a.csv b.csv -pres error -pdf moo.pdf
evince moo.pdf
rm moo.pdf

echo "Expect Error plot Logy: Flatpoint=(0,360) (logy flips -360)"
~/dev/koviz/bin/koviz DP_logy a.csv b.csv -pres error
echo "Expect Error plot Logy: Flatpoint=(0,360) (logy flips -360) (PDF)"
~/dev/koviz/bin/koviz DP_logy a.csv b.csv -pres error -pdf moo.pdf
evince moo.pdf
rm moo.pdf

echo "Expect Error plot Logxy: Empty"
~/dev/koviz/bin/koviz DP_logxy a.csv b.csv -pres error
echo "Expect Error plot Logxy: Empty (PDF)"
~/dev/koviz/bin/koviz DP_logxy a.csv b.csv -pres error -pdf moo.pdf
evince moo.pdf
rm moo.pdf
