#! /usr/bin/env bash

echo "---------------------------------------------------------"
echo "  Test Acssl xls                                         "
echo "---------------------------------------------------------"
echo ""
echo "Action1: View koviz launched"
echo "Expect1: See time, position and velocity"
echo "Action2: Click position" 
echo "Expect2: Units in {m} and points:"
echo "         (0,1),(1,2),(2,5),(3,7),(4,2)"
koviz acssl.xls
