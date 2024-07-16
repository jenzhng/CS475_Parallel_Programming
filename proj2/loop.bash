#!/bin/bash
for t in 4
do
  for n in 100
  do
     g++   proj02.cpp  -NUMT=$t -NUMTRIES=$n  -o proj02  -lm  -fopenmp
    ./proj02
  done
done