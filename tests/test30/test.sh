#! /usr/bin/env bash

echo "-----------------------------------------"
echo "         Test csv -> trk -> csv          "
echo "-----------------------------------------"
echo "Expect: Diff with no differences"
koviz -csv2trk a.csv
mv a.trk b.trk
koviz -trk2csv b.trk
meld a.csv b.csv
rm b.csv b.trk
