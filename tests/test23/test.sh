#! /usr/bin/env bash

echo "-----------------------------------------"
echo "Test error when trying to scale/bias time"
echo "-----------------------------------------"
echo "Data"
cat *.csv
echo ""
echo "map.kvz"
cat map.kvz
echo ""
echo "DP_test"
cat DP_test
echo "---------------------"
echo "Expect error below:"
echo "koviz [error]: var map is attempting to bias time.  Koviz doesn't not support this."
echo "---------------------"

koviz -mapFile map.kvz -timeName "time=a.t=b.t" DP_test .
