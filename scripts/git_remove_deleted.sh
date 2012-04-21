#!/bin/bash

echo "Removing deleted files from git."
for f in $(git ls-files --deleted); do
    #echo "RM $f"
    git rm $f
done
