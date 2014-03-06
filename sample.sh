#!/bin/bash

which clang++ > /dev/null 2>&1
if [[ $? -eq 0 ]]; then
  clang++ sample.cpp -o sample -std=c++11 -stdlib=libc++
else
  g++ sample.cpp -o sample -std=c++11
fi
cat "sample.html" | ./sample
rm sample
