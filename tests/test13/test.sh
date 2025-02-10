#! /usr/bin/env bash

echo "--------------------------------"
echo "Try many runs with single points"
echo "--------------------------------"
cat SET*/RUN*/*.csv
echo "---------------------"
echo ""

echo "Linear: line of points (0-6,0-6) and (0,1),(3,7),(4,0),(5,0)"
~/dev/koviz/bin/koviz -a SET*/RUN*
echo "Linear: line of points (0-6,0-6) and (0,1),(3,7),(4,0),(5,0) (PDF)"
~/dev/koviz/bin/koviz -a SET*/RUN* -pdf moo.pdf
evince moo.pdf
rm moo.pdf

echo "Logx:  Culls out (0,0),(0,1)"
~/dev/koviz/bin/koviz DP_logx SET*/RUN*
echo "Logx:  Culls out (0,0),(0,1) (PDF)"
~/dev/koviz/bin/koviz DP_logx SET*/RUN* -pdf moo.pdf
evince moo.pdf
rm moo.pdf

echo "Logy:  Culls out (0,0),(4,0),(5,0) and goes to Empty"
~/dev/koviz/bin/koviz DP_logy SET*/RUN*
echo "Logy:  Culls out (0,0),(4,0),(5,0) and goes to Empty (PDF)"
~/dev/koviz/bin/koviz DP_logy SET*/RUN* -pdf moo.pdf
evince moo.pdf
rm moo.pdf

echo "Logxy: Culls out (0,0),(4,0),(5,0),(0,1) and goes to Empty"
~/dev/koviz/bin/koviz DP_logxy SET*/RUN*
echo "Logxy: Culls out (0,0),(4,0),(5,0),(0,1) and goes to Empty (PDF)"
~/dev/koviz/bin/koviz DP_logxy SET*/RUN* -pdf moo.pdf
evince moo.pdf
rm moo.pdf
