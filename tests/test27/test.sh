#! /usr/bin/env bash

echo "-----------------------------------------"
echo "-        Test diff time units           -"
echo "-----------------------------------------"
echo ""
echo "Note: Time shift is needed to align time"
echo "      between a.csv and b.csv"
echo ""
echo "Note: DP_test calls for time in hours, so"
echo "      time shift is in hours i.e. -5,-3"
echo ""
echo "Note: Time units are not specified in *.csvs"
echo "      but are seconds in a.csv and minutes "
echo "      in b.csv - as seen in map.kvz"
echo ""
echo "Action: When plot comes up, hit spacebar for error plot"
echo "Expect: (0hr,-100mm),(1,-10),(2,-200),(3,-30),(4,-50)"
koviz DP_test a.csv b.csv -mapFile map.kvz -timeName "time=a.t=b.t" -shift "a.csv:-5,b.csv:-3"
