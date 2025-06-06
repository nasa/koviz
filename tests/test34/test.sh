#! /usr/bin/env bash

echo "---------------------------------------------------"
echo "              Test Envelope                        "
echo "---------------------------------------------------"
echo "# Note: some of the csv data is purposefully off by"
echo "# 0.01 to force -tmt 0.1 (time match tolerance) to "
echo "# align points"
echo ""
echo "Action1: Press 's' for sum"
echo "Expect1: Sum=[(0,7),(1,7),(2,2),(3,12),(4,9),(4.5,9),
                    (5,5),(6,1),(7,7),(8,2),(9,10),(10,7),
		    (11,3),(12,4)]"
echo "Action2: Close plot"
echo "Action3: Select pos"
echo "Action4: Press 'e' for envelope"
echo "Expect4: upper_bound=[(0,6),(1,5),(2,1),(3,5),(4,4)"
echo "                      (4.5,4.5),(5,2),(6,1),(7,4),"
echo "                      (8,1),(9,10),(10,5),(11,3),(12,4)]"
echo ""
echo "         lower_bound=[(0,1),(1,2),(2,1),(3,3),(4,2),"
echo "                      (4.5,2*),(5,1),(6,0),(7,3),(8,1),(9,10),"
echo "                      (10,2),(11,3),(12,4)]"
koviz *.csv -a -tmt 0.1
