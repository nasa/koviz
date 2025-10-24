#! /usr/bin/env bash

echo "---------------------------------------------------------"
echo "  Test -vars2csv and -dp2csv with time conversion        "
echo ""
echo "---------------------------------------------------------"
echo ""
echo "Action1: Look at meld diff"
echo "Expect1: No diffs"
echo "Action2: Look at meld diff"
echo "Expect2: No diffs"
echo "Action3: View stdout in terminal"
echo "Expect3: 0,,"
echo "         koviz [warning]: no values at time=0"
echo "         3,4,3"
echo "         6,,-172"
echo "         15,6004,-637"
echo "         25,11004,"
koviz RUN_test -vars2csv moo.csv -vars "sys.exec.out.time {hr} scale(2) bias(3), a scale(10) bias(4) {cm}, b {cm} scale(-1) bias(3)"
meld moo.csv verif.csv
rm moo.csv
koviz RUN_test DP_test -dp2csv moo.csv
meld moo.csv verif.csv
rm moo.csv
koviz RUN_test -vars2valsAtTime 0  -vars "sys.exec.out.time {hr} scale(2) bias(3), a scale(10) bias(4) {cm}, b {cm} scale(-1) bias(3)"
koviz RUN_test -vars2valsAtTime 3  -vars "sys.exec.out.time {hr} scale(2) bias(3), a scale(10) bias(4) {cm}, b {cm} scale(-1) bias(3)"
koviz RUN_test -vars2valsAtTime 6  -vars "sys.exec.out.time {hr} scale(2) bias(3), a scale(10) bias(4) {cm}, b {cm} scale(-1) bias(3)"
koviz RUN_test -vars2valsAtTime 15 -vars "sys.exec.out.time {hr} scale(2) bias(3), a scale(10) bias(4) {cm}, b {cm} scale(-1) bias(3)"
koviz RUN_test -vars2valsAtTime 25 -vars "sys.exec.out.time {hr} scale(2) bias(3), a scale(10) bias(4) {cm}, b {cm} scale(-1) bias(3)"
