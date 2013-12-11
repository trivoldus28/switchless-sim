#!/usr/bin/python

import sys
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import matplotlib.cm as cm
import matplotlib.mlab as mlab
import numpy as np
import re
import math

def usage () :
    print "Usage: " + sys.argv[0] + " [list of files from parse_output.py]"

def parseCmdArgs () :
    if len(sys.argv[1:]) == 0 :
        print "Invalid number of arguments"
        usage()
        sys.exit (1)
    else :
        return sys.argv[1:]
    
def plotDelay(resultFilenames):
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
 
    maxDelay = 0.0
    for i in range (0, len (delayValues)) :
        for delayValue in delayValues[i] :
            if delayValue > maxDelay :
                maxDelay = delayValue

    colors = cm.rainbow ( np.linspace (0, 1, len (delayValues)))
    plt.hist(delayValues, 20, color=colors, label=resultFilenames)
    plt.legend()
    plt.xlabel ("Packet Delay (ns)")
    plt.ylabel ("Occurrences")
    plt.savefig ("delay.pdf")

def main () :
    # Parse cmd line params
    resultFilenames = parseCmdArgs()
    plotDelay(resultFilenames)
    
if __name__ == "__main__" :
    main()
