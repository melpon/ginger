#!/bin/bash

clang++ main.cpp -o main -std=c++11 -stdlib=libc++
cat "test.html" | ./main
rm main
