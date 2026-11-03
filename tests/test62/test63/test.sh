#! /usr/bin/env bash

echo "-----------------------------------------------"
echo "Test DP unit compatibility and unit overrides"
echo "-----------------------------------------------"
echo ""

echo "-- Test 1 --------------------------"
echo "Action1: View DP_ae"
echo "Action2: Press spacebar twice"
echo "Expect2: Error plot appears"
echo "Expect2: Units are m"
echo "Expect2: First point is (0, -4)"
koviz DP_ae a.csv
echo ""

echo "-- Test 2 --------------------------"
echo "Action1: View DP_ea"
echo "Action2: Press spacebar twice"
echo "Expect2: Error plot appears"
echo "Expect2: Units are cm"
echo "Expect2: First point is (0, 400)"
koviz DP_ea a.csv
echo ""

echo "-- Test 3 --------------------------"
echo "Action1: View DP_amm_e"
echo "Action2: Press spacebar twice"
echo "Expect2: Error plot appears"
echo "Expect2: Units are mm"
echo "Expect2: First point is (0, -4000)"
koviz DP_amm_e a.csv
echo ""

echo "-- Test 4 --------------------------"
echo "Action1: View DP_amm_eft"
echo "Action2: Press spacebar twice"
echo "Expect1: Error plot appears"
echo "Expect1: Units are mm"
echo "Expect1: First point is (0, -4000)"
koviz DP_amm_eft a.csv
echo ""

echo "-- Test 5 --------------------------"
echo "Action1: See koviz with a.csv"
echo "Action2: Select DP_ac"
echo "Action3: Press spacebar"
echo "Expect3: Unit incompatibility message"
echo "Expect3: koviz remains running"
koviz a.csv
echo ""

echo "-- Test 6 --------------------------"
echo "Action1: See koviz with a.csv"
echo "Action2: Select DP_ab"
echo "Action3: Press spacebar"
echo "Expect3: Unit incompatibility message"
echo "Expect3: koviz remains running"
koviz a.csv
echo ""

echo "-- Test 7 --------------------------"
echo "Action1: See koviz with a.csv"
echo "Action2: Select DP_af"
echo "Action3: Press spacebar"
echo "Expect3: Unit incompatibility message"
echo "Expect3: koviz remains running"
koviz a.csv
echo ""

echo "-- Test 8 --------------------------"
echo "Action1: View DP_df"
echo "Action2: Press spacebar twice"
echo "Expect2: Error plot appears"
echo "Expect2: Both variables are unitless"
echo "Expect2: First point is (0, -695)"
koviz DP_df a.csv
