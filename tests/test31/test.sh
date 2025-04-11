#! /usr/bin/env bash

echo "-----------------------------------------"
echo "       Test -filter and -exclude         "
echo "-----------------------------------------"
echo "Test: Filter for a.csv"
echo "Expect: a.x and a.y in vars list"
echo "Action: Click a.y"
echo "Expect: (0,20),(1,40),(2,60),(3,70),(4,80)"
koviz -filter a RUN_test
echo ""
echo "Test: Exclude a.csv which leaves b,c"
echo "Expect: b.x, b.y, c.x, c.y in vars list"
echo "Action: Click c.y"
echo "Expect: (0,23),(1,43),(2,63),(3,73),(4,83)"
koviz -exclude a RUN_test

