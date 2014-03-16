#!/bin/bash

which clang++ > /dev/null 2>&1
if [ $? -eq 0 ]; then
  clang++ test.cpp -o test -std=c++11 -stdlib=libc++
else
  g++ test.cpp -o test -std=c++11
fi
./test
rm test
