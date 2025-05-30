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
echo ""
echo "Test derivatives and integration of velocity"
echo ""
echo "vel(t) = 3*t^2 + 7"
echo "acc(t) = d'vel = 6*t"
echo "jer(t) = d'acc = 6"
echo "pos(t) = integral(vel) = t^3 + 7*t + C=0"
echo ""
echo "Time   Jerk     Acc     Vel     Pos     Absement"
echo "0.0     6.0     0.0     7.0     0.0         0.00"
echo "1.0     6.0     6.0     10.0    8.5         4.25"
echo "2.0     6.0     12.0    19.0    23.0       20.00"
echo "3.0     6.0     18.0    34.0    49.5       56.25"
echo "4.0     6.0     24.0    55.0    94.0      128.00"
echo ""
echo "Action1: Select velocity and then Press i to integrate"
echo "Expect1: {m} {(0,0),(1,8.5),(2,23),(3,49.5),(4,94)}"
echo ""
echo "Action2: Press i again"
echo "Expect2: {m*s} {(0,0),(1,4.25),(2,20),(3,56.25),(4,128)}"
echo ""
echo "Action3: Press 'd' 3 times"
echo "Expect3: {m/s2} {(0,0),(1,6),(2,12),(3,18),(4,24)}"
echo ""
echo "Action4: Press 'd'"
echo "Expect4: {m/s3} {(0,6),(1,6),(2,6),(3,6),(4,6)}"
koviz javpa.csv

echo ""
echo "Test integration with scale/bias/start/stop and xy unit scaling"
echo ""
echo "Action1: Press i to integrate"
echo "Expect1: {--} {(0,0),(1,~8.5),(2,~23),(3,~49.5),(4,~94)}"
echo ""
echo "Action2: Press i again"
echo "Expect2: {m*s} {(0,0),(1,~4.25),(2,~20),(3,~56.25),(4,~128)}"
echo ""
echo "Action3: Press 'd' 3 times"
echo "Expect3: {m/s2} {(0,~0),(1,~6),(2,~12),(3,~18),(4,~24)}"
echo ""
echo "Action4: Press 'd'"
echo "Expect4: {m/s3} {(0,~6),(1,~6),(2,~6),(3,~6),(4,~6)}"
echo "              Line will not be flat due to round off"
koviz DP_test temperature.csv -start 0 -stop 4 -shift -14

echo ""
echo "Test integration with timenames/scale/bias/start/stop and xy unit scaling"
echo ""
echo "To show what's going on in this test, see how two points are aligned at 14hrs:"
echo "cat.csv has point=(2520,7) 2520/60=42hrs 42*DP.scale(0.5)=21hrs 21+shift(-7)=14hrs"
echo "rat.csv has point=(62,7) 62*DP.scale(0.5)=31hrs 31+shift(-17)=14hrs"
echo ""
echo "Action1: Press i to integrate"
echo "Expect1: Both curves {(14,0),(15,8.5),(16,23),(17,49.5),(18,94)}"
echo ""
echo "Action2: Hit spacebar to compare"
echo "Expect2: Flatline"
koviz DP_cat cat.csv rat.csv \
      -timeName "cat.time=rat.time" \
      -start 14 -stop 18 \
      -shift "cat.csv:-7,rat.csv:-17"

echo ""
echo "Test integration constant"
echo ""
echo "This test does a lot of actions then checks a single point"
echo ""
echo "Action1: Select jerk"
echo "Action2: Press i to integrate to acceleration"
echo "Action3: Press i again to integrate to velocity"
echo "Action4: Put 7.0 in integration initial value and hit <enter>"
echo "Action5: Put 0.0 in integration initial value and then click out of box *without* hitting <enter>"
echo "Action6: Press i twice to get to Absement which is iiii(jerk) {m*s}"
echo "Expect6: Last point is (4,128)"
koviz javpa.csv

echo ""
echo "Test integration and changing units"
echo ""
echo "This test changes units while taking derivatives and integrating"
echo ""
echo "Action1: Select velocity"
echo "Action2: Change units to cm/s"
echo "Action3: Press d" 
echo "Expect3: acc(t)=600t => [(0,0),(1,600),(2,1200,(3,1800),(4,2400)] {cm/s2}"
echo "Action4: Press i"
echo "Expect4: Back to original velocity with cm/s"
echo "Action5: Change cm/s to ft/s"
echo "Action6: Press i"
echo "Expect6: [(0,0),(1,27.887),(2,75.459),(162.402),(4,308.39895)]"
echo "Action7: Change {ft} to {m}" 
echo "Action8: Press i"
echo "Expect8: [(0,0),(1,4.25),(2,20),(3,56.25),(4,128)] {m*s}"
koviz javpa.csv

echo ""
echo "Test zooming in to change start/stop"
echo ""
echo "Action1: Mouse zoom in between ~2.5 to ~6.5"
echo "Action2: Press i"
echo "Expect2: [(3,0),(4,3.5),(5,8.0),(6,13.5)]"
echo "Action3: Press d twice"
echo "Expect3: Flat 1 between 3&6 i.e. [(3,1),(4,1),(5,1),(6,1)]"
koviz -a a10.csv

