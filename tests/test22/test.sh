#! /usr/bin/env bash

echo "---------------------------------------------"
echo "a.csv and b.csv have units set in the map.kvz"
echo "map file sets units to {m}"
echo "DP file sets units to {cm}"
echo "---------------------------------------------"
echo "Data"
cat *.csv
echo ""
echo "map.kvz"
cat map.kvz
echo ""
echo "DP_test"
cat DP_test
echo "---------------------"
echo ""
echo "Expect: diamond {cm}=(500,0),(0,500),(-500,0),(0,-500),(500,0)"

koviz -mapFile map.kvz -timeName "time=a.t=b.t" DP_test .
