#! /usr/bin/env bash

echo "-----------------------------------------"
echo "         Test integration                "
echo "-----------------------------------------"
echo "Test empty set a0.csv"
echo "Action: Press i to integrate"
echo "Expect: empty"
koviz a0.csv -a 
echo ""
echo "Test single point a1.csv"
echo "Action: Press i to integrate"
echo "Expect: single point (0,0)"
koviz a1.csv -a 
echo ""
echo "Test two points a2.csv"
echo "Action: Press i to integrate"
echo "Expect: (0,0), (1,2)"
koviz a2.csv -a
echo ""
echo "Test three points a3.csv"
echo "Action: Press i to integrate"
echo "Expect: (0,0),(1,2),(2,7)"
koviz a3.csv -a
echo ""
echo "Test ten points a10.csv"
echo "Action: Press i to integrate"
echo "Expect: (0,0),(1,0.5),(2,2.0),(3,4.5),"
echo "        (4,8.0),(5,12.5),(6,18.0),(7,24.5),"
echo "        (8,32.0),(9,40.5)"
koviz a10.csv -a
