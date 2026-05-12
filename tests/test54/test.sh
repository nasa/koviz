#! /usr/bin/env bash

echo "---------------------------------------------------------"
echo "  Test Trick HDF5                                        "
echo "---------------------------------------------------------"
echo ""
echo "Action1: See co-plot between Trick-HDF5 and Trick-trk"
echo "Expect1: A ball coplot with overlapping/same curves "
echo "Action2: Hit space bar on each plot"
echo "Expect2: Flatline!"
koviz RUN_* DP*
