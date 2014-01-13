#!/usr/bin/python

import sys
import math
import os

FATTREE = 1
MESH = 2
CUBE = 3
CUBEDO = 5
HIERARCHICAL = 4

L4_TCP = 1
L4_UDP = 2

def getXY(numnodes):
  x = math.ceil(math.sqrt(numnodes))
  y = math.ceil(math.sqrt(numnodes))
  # now attempt to reduce the y dimension
  while (x * (y-1)) > numnodes:
      y = y - 1
  return (int(x),int(y))

FANOUT = 48

def getXYZ(numnodes):
    # z is limited to FANOUT (rack depth limitation)
    i = math.ceil(math.pow(numnodes, .333333333))
    if i <= FANOUT:
        x,y,z = (i,i,i)
    else:
        z = FANOUT
        xy = math.ceil(float(numnodes) / z)
        x = math.ceil(math.pow(xy, .5))
        y = x

    # now attempt to reduce the y dimension
    while (x * z * (y-1)) > numnodes:
        y = y - 1

    return (int(x),int(y),int(z))

def getHierarhicalValues(n, t):
  e = 0
  a = 0
  m1 = 1
  m2 = 1
  if t == "lowcost":
    if n <= FANOUT:
      e = 1
    elif n <= FANOUT * FANOUT:
      e = math.ceil(float(n) / FANOUT);
      a = 1
    else:
      e = math.ceil(float(n) / FANOUT);
      a = math.ceil(float(e) / FANOUT);
  elif t == "balanced":
    if n <= FANOUT:
      e = 1
    elif n <= FANOUT * FANOUT:
      e = math.ceil(pow(n,.5));
      a = 1
    else:
      e = math.ceil(pow(n,.66666));
      a = math.ceil(pow(n,.33333));
  elif t == "replicated":
      m1 = 4
      m2 = 4
      if n <= FANOUT:
        e = 1
      elif n <= FANOUT * FANOUT:
        e = math.ceil(pow(n,.5));
        a = n1
      else:
        e = math.ceil(pow(n,.66666));
        a = math.ceil(pow(n,.33333)) * n1;
  return (int(e),int(a),int(m1),int(m2))

def parseStandardVariables(argdict):
  args = ""
  if argdict["topo"] == "fattree":
    args += " --tp=" + `FATTREE`
  elif argdict["topo"] == "mesh":
    x,y = getXY(argdict["nNode"])
    args += " --tp=" + `CUBE`
    args += " --t1=" + `x`
    args += " --t2=" + `y`
    args += " --t3=1"
  elif argdict["topo"] == "cube":
    x,y,z = getXYZ(argdict["nNode"])
    args += " --tp=" + `CUBE`
    args += " --t1=" + `x`
    args += " --t2=" + `y`
    args += " --t3=" + `z`
  elif argdict["topo"] == "cube-dimordered":
    x,y,z = getXYZ(argdict["nNode"])
    args += " --tp=" + `CUBEDO`
    args += " --t1=" + `x`
    args += " --t2=" + `y`
    args += " --t3=" + `z`
  elif argdict["topo"] == "hierarchical":
    hierarchical_type = argdict["hierarchical_type"]
    e,a,m1,m2 = getHierarhicalValues(argdict["nNode"], hierarchical_type)
    args += " --tp=" + `HIERARCHICAL`
    args += " --t1=" + `e`
    args += " --t2=" + `a`
    args += " --t3=" + `m1`
    args += " --t4=" + `m2`

  if argdict["intervaltype"] == "fixed":
    args += " --itype=2"
    if argdict["synctype"] == 1:
      args += " --isize=" + `argdict["interval"]`
    else:
      args += " --isize=" + `argdict["interval"][0]`
      args += " --minint=" + `argdict["interval"][1][0]`
      args += " --maxint=" + `argdict["interval"][1][1]`

  elif argdict["intervaltype"] == "random":
    args += " --itype=1"
    args += " --minint=" + `argdict["interval"][0]`
    args += " --maxint=" + `argdict["interval"][1]`

  args += " --iter=" + `argdict["numiteration"]`
  args += " --sync=" + `argdict["synctype"]`
  args += " --psize=" + `argdict["packetsize"]`

  if argdict["l4type"] == "TCP":
    args += " --l4type=" + `L4_TCP`
  elif argdict["l4type"] == "UDP":
    args += " --l4type=" + `L4_UDP`
  else:
    args += " --l4type=0"

  return args


if __name__ == '__main__':

	try:
		arg1 = sys.argv[1];
	except:
		arg1 = ""

	# unit test
	if (arg1 == 'testxyz'):
	    for i in [2,4,8,16,32,64,100,200,500,1000,2000,5000,10000,15000,20000,25000]:
	        print `i` + ":" + `getXYZ(i)`;
	    exit()


	#major properties, adjust them here
	numberofnodes = [16,64,256,512]
	topologies = ["fattree", "mesh", "cube", "hierarchical", "cube-dimordered"]
	hierarchical_type = "lowcost"
	intervaltypes = ["fixed", "random"]
	synctypes = [1,0] #synchronized or not
	numsenders = [.2,.4,.6,.8]
	numreceivers = [.2,.4,.6,.8] # percentage for random n to random/neighbor m
	neighborlist = [1,3,5,7,9,11,13,15]
	l4types = ["UDP", "TCP"]

	#minor properties
	numiterations = [1,2,3]
	intervals = [1, 5, 10, 20] # in us 
	intervalranges = [[1,5],[10,50]] # in us, used in sporadic interval
	packetsizes = [256,512] # what is this? bytes?

	# OVERRIDE PARAMS HERE
	workload = arg1
	if workload == "":
		workload = "test"

	# numberofnodes = [16]
	# numberofnodes = [64]
	numberofnodes = [256]
	topologies = ["mesh", "cube", "fattree", "hierarchical", "cube-dimordered"]
	intervaltypes = ["fixed"]
	synctypes = [1] #synchronized or not
	# intervaltypes = ["random"]
	# synctypes = [0] #synchronized or not
	packetsizes = [256]
	intervals = [1] # in us 
	# intervals = [10] # in us 
	intervalranges = [[0,1]] # in us, used in sporadic interval
	# intervalranges = [[1,10]] # in us, used in sporadic interval
	numiterations = [1]
	neighborlist = [4]
	# numiterations = [16]
	numsenders = [1]
	numreceivers = [1]
	l4types = ["TCP"]

	namesuffix = `numberofnodes[0]` + "_neighbor"

	# if workload == "all-to-all":
	# 	numsenders = [1]
	# 	numreceivers = [1]

	if workload == "rnnm":
		numreceivers = neighborlist

	commands = []
	logfnames = []

	for nNode in numberofnodes:
		for numsender in numsenders:
			for numreceiver in numreceivers:
				for topo in topologies:
					for numiteration in numiterations:
						for synctype in synctypes:
							for intervaltype in intervaltypes:
								intervalsmux = []
								if (intervaltype == "fixed" and synctype == 1):
									intervalsmux = intervals
								elif intervaltype == "fixed" and synctype == 0:
									intervalsmux = zip(intervals,intervalranges)
								elif intervaltype == "random":
									intervalsmux = intervalranges
								else: assert(0)
								for interval in intervalsmux:
										for packetsize in packetsizes:
											for l4type in l4types:
												argdict = {}
												argdict["nNode"] = nNode
												argdict["topo"] = topo
												argdict["hierarchical_type"] = hierarchical_type
												argdict["intervaltype"] = intervaltype
												argdict["interval"] = interval
												# argdict["intervalrange"] = intervalrange
												argdict["synctype"] = synctype
												argdict["packetsize"] = packetsize
												argdict["numiteration"] = numiteration 
												argdict["l4type"] = l4type

												# if (intervaltype == "fixed"):
												# 	# skip the interval ranges iterations
												# 	if intervalrange != intervalranges[0]:
												# 		continue
												# elif (intervaltype == "random"):
												# 	if interval != intervals[0]:
												# 		continue

												args = parseStandardVariables(argdict)

												logfname = ""
												intervallog = ""
												try:
													intervallog = `interval[0]` + "_" + `interval[1][0]` + "_" + `interval[1][1]`
												except:
													try:
														intervallog = `interval[0]` + "_" + `interval[1]`
													except:
														intervallog = `interval`

												if (workload == "all-to-all" or workload == "test"):
													args += " --ncount=" + `nNode`
													args += " --scount=" + `nNode`
													args += " --rcount=" + `nNode-1`
													logfname = "log_alltoall_" + `nNode`
												
												if (workload == "rnrm"):
													# random n, random m
													nsender = int(numsender * nNode)
													nreceiver = int(numreceiver * nNode)
													args += " --scount=" + `nsender`
													args += " --rcount=" + `nreceiver`
													args += " --ncount=" + `nNode`
													logfname = "log_rnrm_" + `nNode` + "_" + `nsender` + "_" + `nreceiver`

												if workload == "rnnm":
													# random n, neighbor m
													nsender = int(numsender * nNode)
													nneighbor = numreceiver
													args += " --scount=" + `nsender`
													args += " --neighborcount=" + `nneighbor`
													args += " --ncount=" + `nNode`
													logfname = "log_rnnm_" + `nNode` + "_" + `nsender` + "_" + `nneighbor`

												# logfname += "_" + topo + "_" + intervaltype + "_" + intervallog + "_" + `synctype` \
												#  			 + "_" + `packetsize` + "_" + `numiteration`

												logfname = topo
												logfname += ".log"

												command = './waf --run "main-test' + args + ' --debug=1"'
												commands.append(command)
												logfnames.append(logfname)
												# print(command)

	import parse_output
	from cStringIO import StringIO

	execute = False
	writetofile = False

  # execute = True # uncomment this line to execute the command)
	# writetofile = True

	filestoplot = []
	for i,command in enumerate(commands):
		if writetofile:
			command += " 2> " + logfnames[i]
		print(command)
		# print(logfnames[i])
		if execute:
			os.system(command) 
			sys.stdout = parseout = StringIO()
			parse_output.parseOutput(logfnames[i])
			sys.stdout = sys.__stdout__
			# print(parseout.getvalue()) # UNCOMMENT TO SEE THE PARSED OUTPUT
			parsedf = logfnames[i] + ".parsed"
			f = open(parsedf,'w')
			f.write(parseout.getvalue())
			filestoplot.append(parsedf)
			f.close()

	if execute:
		import plot_delay
		plot_delay.plotDelay(filestoplot)	
		import plot_latency
		plot_latency.plotLatency(filestoplot)

		os.system("mv delay.pdf delay" + "_" + `numberofnodes[0]` + "_" + namesuffix + "pdf")
		os.system("mv latency.pdf latency" + "_" + `numberofnodes[0]` + "_" + namesuffix + "pdf")




