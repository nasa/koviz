#! /usr/bin/env bash

echo "-----------------------------------------------"
echo "Test mixing log/linear and ranging across pages"
echo "-----------------------------------------------"
echo ""

echo "-- Test 1 --------------------------"
echo "Action1: View DP_range_linlin tab"
echo "Expect1: X range is between ~2.5 and ~3.5"
echo "Expect1: And rooftop red/blue from 4000->5000->3500"
echo "Action2: Click DP_norange_linlin tab"
echo "Expect2: X range is between ~2.5 and ~3.5"
echo "Expect2: Y-auto range not done but still has rooftop"
koviz DP_range_linlin DP_norange_linlin a.csv b.csv
echo ""

echo "-- Test 2 --------------------------"
echo "Action1: View DP_norange_linlin tab"
echo "Expect1: X range between 0 and 5"
echo "Action2: Click DP_range_linlin tab"
echo "Expect2: X range between 2.5 and 3.5"
echo "Expect2: And rooftop red/blue from 4000->5000->3500"
koviz DP_norange_linlin DP_range_linlin a.csv b.csv
echo ""

echo "-- Test 3 --------------------------"
echo "Action1: View DP_range_linlin tab"
echo "Expect1: X range between 2.5 and 3.5"
echo "Action2: Click DP_norange_loglin tab"
echo "Expect2: X range between 10^0.4 and past tic 10^0.5"
koviz DP_range_linlin DP_norange_loglin a.csv b.csv
echo ""

echo "-- Test 4 --------------------------"
echo "Action1: View DP_norange_linlin tab"
echo "Expect1: X range between 0 and 5"
echo "Action2: Click DP_norange_loglin tab"
echo "Expect2: X range between 1 (due to cull) and 5 (past 10^.6)"
koviz DP_norange_linlin DP_norange_loglin a.csv b.csv
echo ""


echo "-- Test 5 --------------------------"
echo "Action1: View DP_range_loglin tab"
echo "Expect1: X range between 10^0.4 and past 10^0.5"
echo "Expect1: Roof top red/blue peaking at 5000"
echo "Action2: Click DP_norange_loglin tab"
echo "Expect2: X range between 10^0.4 and past 10^0.5"
echo "Expect2: Flatter roof top red/blue peaking at 5000"
koviz DP_range_loglin DP_norange_loglin a.csv b.csv
echo ""

echo "-- Test 6 --------------------------"
echo "Action1: View DP_range_loglin tab"
echo "Expect1: X range between 10^0.4 and past 10^0.5"
echo "Expect1: Roof top red/blue peaking at 5000"
echo "Action2: Click DP_range_linlin tab"
echo "Expect2: X range between 2.5 and 3.5"
echo "Expect2: Same roof top red/blue peaking at 5000 with x linear scale"
koviz DP_range_loglin DP_range_linlin a.csv b.csv
