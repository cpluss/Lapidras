#!/bin/bash

for f in $(git ls-files --deleted); do
    echo "RM $f"
    git rm $f
done
