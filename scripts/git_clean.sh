#!/bin/zsh

git ls-files | grep '\.swp$' | xargs git rm
for f in $(git ls-files --deleted); do
    #echo "RM $f"
    git rm $f
done

#git clean -f
git add -u *
