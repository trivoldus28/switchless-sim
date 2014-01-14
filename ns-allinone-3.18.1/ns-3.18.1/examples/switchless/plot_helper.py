#!/usr/bin/python

import sys
import math
import os
from cStringIO import StringIO

import parse_output

if __name__ == '__main__':
	files = sys.argv[1:]
	print "Plotting " + `files`

	filestoplot = []

	for f in files:
		# os.system(command) 
		parsedf = f + ".parsed"
		filestoplot.append(parsedf)
		if os.path.isfile(parsedf) == False:
			sys.stdout = parseout = StringIO()
			parse_output.parseOutput(f)
			sys.stdout = sys.__stdout__
			# print(parseout.getvalue()) # UNCOMMENT TO SEE THE PARSED OUTPUT
			f = open(parsedf,'w')
			f.write(parseout.getvalue())
			f.close()

	import plot_delay
	plot_delay.plotDelay(filestoplot)	
	import plot_latency
	plot_latency.plotLatency(filestoplot)

	# os.system("mv delay.pdf delay" + "_" + `numberofnodes[0]` + "_" + namesuffix + "pdf")
	# os.system("mv latency.pdf latency" + "_" + `numberofnodes[0]` + "_" + namesuffix + "pdf")
	os.system("mv delay.pdf " + filestoplot[0] + "_delay.pdf")
	os.system("mv latency.pdf " + filestoplot[0] + "_latency.pdf")
