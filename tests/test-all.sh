#! /usr/bin/env bash

for dir in $(ls -d test*/ | sort -V); do
    echo "----------------------------------------------------------------"
    echo "${dir}test.sh..."
    if [[ -x "${dir}test.sh" ]]; then
        (cd "$dir" && ./test.sh)
    else
        echo "No executable test.sh found in $dir, skipping..."
    fi
done

