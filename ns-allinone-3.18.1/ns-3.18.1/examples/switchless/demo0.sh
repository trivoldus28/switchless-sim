#!/bin/bash

./waf --run "main-test --tp=2 --t1=2 --t2=2 --itype=2 --isize=20 --minint=0 --maxint=1 --iter=2 --sync=1 --psize=256 --ncount=4 --scount=4 --rcount=3" 
