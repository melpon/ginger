#!/bin/bash

which clang++ > /dev/null 2>&1
if [[ $? -eq 0 ]]; then
  CC=clang++
else
  CC=g++
fi
$CC test.cpp -o test -std=c++11 -stdlib=libc++
cat "test.html" | ./test
rm test
