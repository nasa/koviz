#! /usr/bin/env bash

echo "---------------------------------------------------------"
echo "  Test Parquet                                           "
echo "---------------------------------------------------------"
echo ""
echo "Action1: View koviz launched"
echo "Expect1: See time, position and velocity"
echo "Action2: Click position" 
echo "Expect2: 10 stacked RUN curves"
echo "Action3: Click top curve" 
echo "Expect3: init=(0,10000)"
echo "         last=(19,10019)"
koviz -runColumnName run monte_test.parquet
