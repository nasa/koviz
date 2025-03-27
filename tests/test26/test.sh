#! /usr/bin/env bash

echo "-----------------------------------------"
echo "-        Test Start/Stop Time           -"
echo "-----------------------------------------"
echo "Action: Put 103 and 114 for start/stop time"
echo "Expect: (104,1006),(106,1009),(108,1012),"
echo "(110,1015),(112,1018),(114,1021)"
koviz a.csv DP_test
