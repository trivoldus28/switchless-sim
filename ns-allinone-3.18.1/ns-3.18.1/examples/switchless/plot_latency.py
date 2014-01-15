#!/usr/bin/python

import sys
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import matplotlib.cm as cm
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
                latencies.append (float(line[9:-1])/1000.0)
                foundLatency = True
                break
        
        if not foundLatency :
            print "Latency value not found in file '" + resultFilename + "'"
            sys.exit(1)

    matplotlib.rcParams.update({'font.size':17})
    matplotlib.rcParams.update({'figure.autolayout': True})
    
    # Plot latencies
    x = np.arange (len (latencies))
    plt.bar (x + 0.5, latencies, color='gray', width=0.3)
    shortenedfilenames = []
    for name in resultFilenames:
        shortenedfilenames.append(name.split('.')[0])
    plt.xticks (x + 0.65, shortenedfilenames, rotation=45, ha='right')
    plt.xlim(xmin=0)
    plt.ylabel ("Latency (us)")
    plt.savefig ("latency.pdf", cmap=cm.Greys_r)
    plt.close()
    
def main () :
    # Parse cmd line params
    resultFilenames = parseCmdArgs()
    plotLatency(resultFilenames)

if __name__ == "__main__" :
    main()
