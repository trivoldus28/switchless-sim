switchless-sim
==============
Simulator for switchless project

SETUP
=====
`cd ns-*/;`
`./build.py`
`CXXFLAGS='-O2 --std=gnu++0x -g -I../examples/HadoopSim' ./waf configure -d debug --enable-examples`
`./waf`

RUN
===
Use simulate.py to generate run commands.
Eg. For the "test" configuration, just run
`python simulate.py test`

Here are some sample command line commands for the lazies (all are around 256 nodes):
**Mesh** *16x16*
`./waf --run "main-test --tp=2 --t1=16 --t2=16 --itype=2 --isize=1 --iter=1 --sync=1 --psize=256 --ncount=256 --scount=256 --rcount=255 --debug=1"`
**Cube** *7x6x7*
`./waf --run "main-test --tp=3 --t1=7 --t2=6 --t3=7 --itype=2 --isize=1 --iter=1 --sync=1 --psize=256 --ncount=256 --scount=256 --rcount=255 --debug=1"`
**Fat-tree**
`./waf --run "main-test --tp=1 --itype=2 --isize=1 --iter=1 --sync=1 --psize=256 --ncount=256 --scount=256 --rcount=255 --debug=1"`
**Hierarchical** *low-cost*
`./waf --run "main-test --tp=4 --t1=6 --t2=1 --t3=0 --t4=0 --itype=2 --isize=1 --iter=1 --sync=1 --psize=256 --ncount=256 --scount=256 --rcount=255 --debug=1"`
