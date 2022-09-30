#!/bin/bash

# https://gist.github.com/cmpark0126/459514e8e73cac0c76284b8ec47334a7

name=$(basename ${1%.*c}) # detatch .c from c file name

clang -O0 -Xclang -disable-O0-optnone -fno-discard-value-names -emit-llvm -c $1 -o out/bc/$name.bc
llvm-dis out/bc/$name.bc -o out/ll/$name.ll
