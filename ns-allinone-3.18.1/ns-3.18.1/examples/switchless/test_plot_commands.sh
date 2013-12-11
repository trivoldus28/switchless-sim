#!/bin/bash

# Mesh
./waf --run "main-test --tp=2 --t1=4 --t2=4 --itype=1 --minint=0 --maxint=1 --iter=3 --sync=0 --psize=256 --ncount=16 --scount=16 --rcount=15 --debug=1" > test_mesh.out 2>&1

# Fat-Tree
./waf --run "main-test --tp=1 --t1=8 --t2=2 --itype=1 --minint=0 --maxint=1 --iter=3 --sync=0 --psize=256 --ncount=16 --scount=16 --rcount=15 --debug=1" > test_tree.out 2>&1

# Cube
./waf --run "main-test --tp=3 --t1=2 --t2=4 --itype=1 --minint=0 --maxint=1 --iter=3 --sync=0 --psize=256 --ncount=16 --scount=16 --rcount=15 --debug=1" > test_cube.out 2>&1

# Parse output
./parse_output.py test_mesh.out > test_mesh.parsed
./parse_output.py test_tree.out > test_tree.parsed
./parse_output.py test_cube.out > test_cube.parsed

# Plot latency
./plot_latency.py test_mesh.parsed test_tree.parsed test_cube.parsed
mv latency.pdf test_latency.pdf

# Plot delay
./plot_delay.py test_mesh.parsed test_tree.parsed test_cube.parsed
mv delay.pdf test_delay.pdf
