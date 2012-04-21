#!/bin/bash

 git ls-files | grep '\.swp$' | xargs git rm
