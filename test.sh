#!/bin/bash

clang++ test.cpp -o test -std=c++11 -stdlib=libc++
cat "test.html" | ./test
rm test
