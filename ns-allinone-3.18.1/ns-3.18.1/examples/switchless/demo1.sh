#!/bin/bash

./waf --run "main-test --tp=2 --t1=2 --t2=2 --itype=1 --minint=0 --maxint=10 --iter=2 --sync=0 --psize=256 --ncount=4 --scount=4 --rcount=3" 
