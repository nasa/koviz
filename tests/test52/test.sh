#! /usr/bin/env bash

echo "---------------------------------------------------------"
echo "  Test -vars2valsAtTime                                  "
echo "                                                         "
echo "---------------------------------------------------------"
echo ""
echo "Action1: cd ~/dev/sims/Ball/SIM_ball_L2 ; #2"
echo "Expect1: N/A"
echo "Action2: koviz RUN_test -vars2valsAtTime 125.6 -vars \"ball.state.out.position[0] {cm} bias(100) scale(2),ball.state.out.velocity[0] {mph}\""
echo "Expect2: -103.758,0.17872"
