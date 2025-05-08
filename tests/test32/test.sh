#! /usr/bin/env bash

echo "-------------------------------------------"
echo "       Test -session option                "
echo "-------------------------------------------"
echo ""
echo "Expect1: Brown,purple,orange dog leg lines"
echo "Expect2: Page title=Page Session Test"
echo "Expect3: t2=Title Two t3=Title Three"
echo "Expect4: Satellite in orange"
echo "Expect5: Satellite [(0,23),(1,43),(2,63),"
echo "                    (3,73),(4,83)]"
echo "Expect6: Legend labels brown,purple,orange"
echo "         Satellite not on legend label :("
koviz -session session
