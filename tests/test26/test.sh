#! /usr/bin/env bash

echo "-----------------------------------------"
echo "-        Test Start/Stop Time           -"
echo "-----------------------------------------"
echo "Put 103 and 114 for start/stop time"
echo "Expect: (100,1000),(102,1003),(104,1006)"
echo "        (106,1009),(108,1012),(110,1015)"
echo "        (112,1018),(114,1021),(116,1024)"
echo "        (118,1027)"
koviz a.csv DP_test
