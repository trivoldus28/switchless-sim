#!/usr/bin/python

import sys
import math
import os

TREE = 1
MESH = 2
CUBE = 3

def getXY(numnodes):
	if (math.sqrt(numnodes) % 1):
		# not a perfect square root, hopefully this works
		x = math.sqrt(numnodes / 2) * 2
		y = math.sqrt(numnodes / 2)
	else:
		x = math.sqrt(numnodes)
		y = math.sqrt(numnodes)
	return (int(x),int(y))

def getMN(numnodes):
	if numnodes == 2: return (2,1)
	elif numnodes == 4: return (2,2)
	elif numnodes == 8: return (2,3)
	elif numnodes == 16: return (2,4)
	elif numnodes == 32: return (2,5)
	elif numnodes == 64: return (4,3)
	elif numnodes == 128: return (2,7)
	elif numnodes == 256: return (4,4)
	elif numnodes == 512: return (2,9)
	elif numnodes == 1024: return (4,5)
	elif numnodes == 2048: return (2,11)
	elif numnodes == 4096: return (8,4)
	else: assert(0)

def parseStandardVariables(argdict):
	args = ""
	if argdict["topo"] == "tree":
		machineperrack = 8 # might want to parameterize these
		fanout = 2
		args += " --tp=" + `TREE`
		args += " --t1=" + `machineperrack`
		args += " --t2=" + `fanout`
	elif argdict["topo"] == "mesh":
		x,y = getXY(argdict["nNode"])
		args += " --tp=" + `MESH`
		args += " --t1=" + `y`
		args += " --t2=" + `x`
	elif argdict["topo"] == "cube":
		m,n = getMN(argdict["nNode"])
		args += " --tp=" + `CUBE`
		args += " --t1=" + `m`
		args += " --t2=" + `n`

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

	return args


if __name__ == '__main__':

	#major properties, adjust them here
	numberofnodes = [16,64,256,512]
	topologies = ["tree", "mesh", "cube"]
	intervaltypes = ["fixed", "random"]
	synctypes = [1,0] #synchronized or not
	numsenders = [.2,.4,.6,.8]
	numreceivers = [.2,.4,.6,.8] # percentage for random n to random/neighbor m
	neighborlist = [1,3,5,7,9,11,13,15]

	#minor properties
	numiterations = [1,2,3]
	intervals = [1, 5, 10, 20] # in us 
	intervalranges = [[1,5],[10,50]] # in us, used in sporadic interval
	packetsizes = [256,512]

	# OVERRIDE PARAMS HERE
	try:
		workload = sys.argv[1]
	except:
		workload = "test"

	if workload == "all-to-all":
		numsenders = [1]
		numreceivers = [1]

	if workload == "test":
		numberofnodes = [16]
		topologies = ["mesh", "cube"]
		intervaltypes = ["fixed"]
		synctypes = [0] #synchronized or not
		packetsizes = [256]
		intervals = [20] # in us 
		intervalranges = [[0,1]] # in us, used in sporadic interval
		numiterations = [3]
		numsenders = [1]
		numreceivers = [1]

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
											argdict = {}
											argdict["nNode"] = nNode
											argdict["topo"] = topo
											argdict["intervaltype"] = intervaltype
											argdict["interval"] = interval
											# argdict["intervalrange"] = intervalrange
											argdict["synctype"] = synctype
											argdict["packetsize"] = packetsize
											argdict["numiteration"] = numiteration 

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

											logfname += "_" + topo + "_" + intervaltype + "_" + intervallog + "_" + `synctype` \
											 			 + "_" + `packetsize` + "_" + `numiteration`
											logfname += ".log"

											command = './waf --run "main-test' + args + ' --debug=1"'
											commands.append(command)
											logfnames.append(logfname)
											# print(command)

	import parse_output
	from cStringIO import StringIO

	parsedfiles = []

	writetofile = False
	writetofile = True

	for i,command in enumerate(commands):
		if writetofile:
			command += " 2> " + logfnames[i]
		print(command)
		# print(logfnames[i])
		os.system(command) # uncomment this line to execute the command)
		sys.stdout = parseout = StringIO()
		parse_output.parseOutput(logfnames[i])
		sys.stdout = sys.__stdout__
		# print(parseout.getvalue()) # UNCOMMENT TO SEE THE PARSED OUTPUT
		parsedf = logfnames[i] + ".parsed"
		f = open(parsedf,'w')
		f.write(parseout.getvalue())
		parsedfiles.append(parsedf)

	import plot_delay
	plot_delay.plotDelay(parsedfiles)	
	import plot_latency
	plot_latency.plotLatency(parsedfiles)




