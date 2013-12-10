#!/usr/bin/python

import sys
import math

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
		args += " --itype=1"
		args += " --isize=" + `argdict["intervals"][0]`
	elif argdict["intervaltype"] == "random":
		args += " --itype=2"
		args += " --minint=" + `argdict["intervalranges"][0][0]`
		args += " --maxint=" + `argdict["intervalranges"][0][1]`
	args += " --sync=" + `argdict["synctype"]`
	# args += " --p"=#MISSING ARGUMENT FOR NUMBER OF INTERVALS
	args += " --psize=" + `argdict["packetsize"]`

	return args


if __name__ == '__main__':
	workload = sys.argv[1]
	if workload == "all-to-all":
		numberofnodes = [16,32,64,128,256,512,1024]
		topologies = ["tree", "mesh", "cube"]
		intervaltypes = ["fixed", "random"]
		synctypes = [1,0] #synchronized or not
		intervals = [5,10,20,40,80] # in ms 
		intervalranges = [[5,80]] # in ms, used in sporadic interval

		#minor properties
		numberofpackets = [1]
		packetsizes = [512]

		for nNode in numberofnodes:
			for topo in topologies:
				for intervaltype in intervaltypes:
					for synctype in synctypes:
						# for interval in intervals:
							# for intervalrange in intervalranges:
								# for numpacket in numberofpackets: # MISSING ARGUMENT IN PROGRAM
									for packetsize in packetsizes:
										argdict = {}
										argdict["nNode"] = nNode
										argdict["topo"] = topo
										argdict["intervaltype"] = intervaltype
										argdict["intervals"] = intervals
										argdict["intervalranges"] = intervalranges
										argdict["synctype"] = synctype
										argdict["packetsize"] = packetsize
										args = parseStandardVariables(argdict)

										#all-to-all specific parameter
										args += " --ncount=" + `nNode`
										args += " --scount=" + `nNode`
										args += " --rcount=" + `nNode`

										log = ""

										command = './waf --run "main-test' + args + '"'
										print(command)

	if workload == "test":
		numberofnodes = [16]
		topologies = ["mesh"]
		intervaltypes = ["fixed"]
		synctypes = [1] #synchronized or not
		intervals = [20] # in ms 
		intervalranges = [[5,80]] # in ms, used in sporadic interval

		#minor properties
		numberofpackets = [1]
		packetsizes = [512]

		for nNode in numberofnodes:
			for topo in topologies:
				for intervaltype in intervaltypes:
					for synctype in synctypes:
						# for interval in intervals:
							# for intervalrange in intervalranges:
								# for numpacket in numberofpackets: # MISSING ARGUMENT IN PROGRAM
									for packetsize in packetsizes:
										argdict = {}
										argdict["nNode"] = nNode
										argdict["topo"] = topo
										argdict["intervaltype"] = intervaltype
										argdict["intervals"] = intervals
										argdict["intervalranges"] = intervalranges
										argdict["synctype"] = synctype
										argdict["packetsize"] = packetsize
										args = parseStandardVariables(argdict)

										#all-to-all specific parameter
										args += " --ncount=" + `nNode`
										args += " --scount=" + `nNode`
										args += " --rcount=" + `nNode`

										log = ""

										command = './waf --run "main-test' + args + '"'
										print(command)

	if workload == "random-n-to-random-m":
		# covers
		# random n to m
		# one to random m
		# n to one
		numberofnodes = [16,32,64,128,256,512,1024]
		topologies = ["tree", "mesh", "cube"]
		intervaltypes = ["fixed", "random"]
		synctypes = [1,0] #synchronized or not
		intervals = [5,10,20,40,80] # in ms 
		intervalranges = [[5,80]] # in ms, used in sporadic interval
		numsenders = [.2,.4,.6,.8]
		numreceivers = [.2,.4,.6,.8] # percentage

		#minor properties
		numberofpackets = [1]
		packetsizes = [512]

		for nNode in numberofnodes:
			for numsender in numsenders:
				for numreceiver in numreceivers:
					for topo in topologies:
						for intervaltype in intervaltypes:
							for synctype in synctypes:
								# for interval in intervals:
									# for intervalrange in intervalranges:
										# for numpacket in numberofpackets: # MISSING ARGUMENT IN PROGRAM
											for packetsize in packetsizes:
												argdict = {}
												argdict["nNode"] = nNode
												argdict["topo"] = topo
												argdict["intervaltype"] = intervaltype
												argdict["intervals"] = intervals
												argdict["intervalranges"] = intervalranges
												argdict["synctype"] = synctype
												argdict["packetsize"] = packetsize
												args = parseStandardVariables(argdict)

												#random n to m specific parameter
												nsender = int(numsender * nNode)
												nreceiver = int(numreceiver * nNode)
												args += " --scount=" + `nsender`
												args += " --rcount=" + `nreceiver`
												args += " --ncount=" + `nNode`

												log = ""

												command = './waf --run "main-test' + args + '"'
												print(command)

	if workload == "random-n-to-m-neighbors":
		# covers
		# all to set m
		# n to set m
		numberofnodes = [16,32,64,128,256,512,1024]
		topologies = ["tree", "mesh", "cube"]
		intervaltypes = ["fixed", "random"]
		synctypes = [1,0] #synchronized or not
		intervals = [5,10,20,40,80] # in ms 
		intervalranges = [[5,80]] # in ms, used in sporadic interval
		numsenders = [.2,.4,.6,.8,1]
		numneighbors = [1,2,4,8,16,32] # percentage

		#minor properties
		numberofpackets = [1]
		packetsizes = [512]

		for nNode in numberofnodes:
			for numsender in numsenders:
				for numneighbor in numneighbors:
					for topo in topologies:
						for intervaltype in intervaltypes:
							for synctype in synctypes:
								# for interval in intervals:
									# for intervalrange in intervalranges:
										# for numpacket in numberofpackets: # MISSING ARGUMENT IN PROGRAM
											for packetsize in packetsizes:
												argdict = {}
												argdict["nNode"] = nNode
												argdict["topo"] = topo
												argdict["intervaltype"] = intervaltype
												argdict["intervals"] = intervals
												argdict["intervalranges"] = intervalranges
												argdict["synctype"] = synctype
												argdict["packetsize"] = packetsize
												args = parseStandardVariables(argdict)

												#specific parameter
												nsender = int(numsender * nNode)
												nneighbor = numneighbor
												args += " --scount=" + `nsender`
												args += " --neighborcount=" + `nneighbor`
												args += " --ncount=" + `nNode`

												log = ""

												command = './waf --run "main-test' + args + '"'
												print(command)




