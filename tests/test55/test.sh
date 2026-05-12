#! /usr/bin/env bash

echo "---------------------------------------------------------"
echo "  Test Generic HDF5                                      "
echo "---------------------------------------------------------"
echo ""
echo "Action1: View koviz launched"
echo "Expect1: List of /Storage vars and time (at bottom)"
echo "Action2: Click /Storage/dynamics/position_y"
echo "Expect2: Half parabola (0,0)->(10,100)"
koviz koviz_generic.h5 -timeName "time"
