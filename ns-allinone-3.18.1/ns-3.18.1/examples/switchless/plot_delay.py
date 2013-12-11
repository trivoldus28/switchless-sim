#!/usr/bin/python

import sys
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import matplotlib.cm as cm
import numpy as np
import re

def usage () :
    print "Usage: " + sys.argv[0] + " [list of files from parse_output.py]"

def parseCmdArgs () :
    if len(sys.argv[1:]) == 0 :
        print "Invalid number of arguments"
        usage()
        sys.exit (1)
    else :
        return sys.argv[1:]
    
def main () :
    # Parse cmd line params
    resultFilenames = parseCmdArgs()
    
    # Size-Delay values from each file
    sizeValues = []
    delayValues = []

    # Loop over files and parse size=delay values
    for resultFilename in resultFilenames :
        # Append a new array for this file's size-delay values
        sizeValues.append ([])
        delayValues.append ([])
        index = len (sizeValues) - 1
        resultFile = open (resultFilename)
        
        inSizeDelayBlock = False
        for line in resultFile :
            if not inSizeDelayBlock :
                if "(Packet Size, Delay) Values:" in line :
                    inSizeDelayBlock = True
            else :
                if re.search ("\([-+]?[0-9]*\.?[0-9]+, [-+]?[0-9]*\.?[0-9]+\)", line) :
                    splitLine = line.split (",")
                    packetSize = int (splitLine[0][1:])
                    delay = float (splitLine[1][1:-2])
                    sizeValues[index].append (packetSize)
                    delayValues[index].append (delay)
                else :
                    inSizeDelayBlock = False
                
    
    # Make scatter plot
    colors = cm.rainbow ( np.linspace (0, 1, len (sizeValues)))
    for i in range(0, len (sizeValues)) :
        plt.scatter (sizeValues[i], delayValues[i], color=colors[i])
    plt.yscale ('log')
    plt.ylim(ymin=0)
    plt.xlim(xmin=-10)
    plt.legend (resultFilenames)
    plt.xlabel ("Packet Size (bytes)")
    plt.ylabel ("Packet Delay (ns) (Log Scale)")
    plt.savefig ("delay.pdf")
    

if __name__ == "__main__" :
    main()
