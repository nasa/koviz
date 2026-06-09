#! /usr/bin/env bash

echo "---------------------------------------------------------"
echo "  Test DP Comments                                       "
echo "---------------------------------------------------------"
echo ""
echo "Action1: View koviz launched"
echo "Expect1: Velocity in m/s"
echo "Expect2: Top plot has # Not A Comment"
echo "Expect3: Bottom plot title is ###"
koviz DP_comments a.csv 
