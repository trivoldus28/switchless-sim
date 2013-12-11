#!/usr/bin/python

import sys
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import numpy as np

def usage () :
    print "Usage: " + sys.argv[0] + " [list of files from parse_output.py]"

def parseCmdArgs () :
    if len(sys.argv[1:]) == 0 :
        print "Invalid number of arguments"
        usage()
        sys.exit (1)
    else :
        return sys.argv[1:]

def plotLatency(resultFilenames):
    # Latency values from each file
    latencies = []

    # Loop over files and parse latency value
    for resultFilename in resultFilenames :
        resultFile = open (resultFilename)
        
        foundLatency = False
        for line in resultFile :
            if "Latency: " in line :
                latencies.append (float(line[9:-1]))
                foundLatency = True
                break
        
        if not foundLatency :
            print "Latency value not found in file '" + resultFilename + "'"
            sys.exit(1)
    
    # Plot latencies
    x = np.arange (len (latencies))
    plt.bar (x, latencies)
    shortenedfilenames = []
    for name in resultFilenames:
        shortenedfilenames.append(name.split('.')[0])
    plt.xticks (x + 0.4, shortenedfilenames)
    plt.ylabel ("Latency (ns)")
    plt.savefig ("latency.pdf")
    plt.close()
    
def main () :
    # Parse cmd line params
    resultFilenames = parseCmdArgs()
    plotLatency(resultFilenames)

if __name__ == "__main__" :
    main()
