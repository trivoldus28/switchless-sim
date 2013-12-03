switchless-sim
==============

Simulator for switchless project

SETUP
`
cd ns-*/
./build.py
CXXFLAGS='-O2 --std=gnu++0x -g -I../examples/HadoopSim' ./waf configure -d debug --enable-examples
./waf
`

RUN
`
./waf --run "HadoopSim 0 0 0 1 examples/HadoopSim/bench-trace/star.nettopo examples/HadoopSim/bench-trace/bayes/Trace 11 0 examples/HadoopSim/debug-output/ 10 10 1000 1000 0"
`

The different parameters are, in order and cannot be omitted

jobSubmitTYpe[0-replay, 1-serial, 2-stress] 

schedType[0-default, 1-netOpt] 

topoType[0-star, 1-rackrow, 2-fattree] 

nodesPerRack (1 is probably not right)

topologyFile 

traceFilePrefix 

numTraceFiles 

needDebug 

debugDir 

scaledMapCPUTime(ms) (10ms is probably not right)

scaledDownRatioForReduce (10ms is probably not right)

customMapNum (1000 is probably not right)

customReduceNum (1000 is probably not right)

needDataImport (1 causes error?)


