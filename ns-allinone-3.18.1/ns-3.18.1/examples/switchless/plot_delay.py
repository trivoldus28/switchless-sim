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
 
    # Calculate the max and mean
    maxDelays = []
    means = []
    for i in range (0, len (delayValues)) :
        sum = 0
        maxDelay = 0.0
        for delayValue in delayValues[i] :
            sum += delayValue
            if delayValue > maxDelay :
                maxDelay = delayValue
        means.append (sum / len (delayValues[i]))
        maxDelays.append (maxDelay)

    # Calculate standard deviations
    stdDevs = []
    for i in range (0, len (delayValues)) :
        differenceSumsSquared = 0
        for delayValue in delayValues[i] :
            difference = delatValue = means[i]
            differenceSumsSquared += (delayValue * delayValue)
        stdDevs.append (math.sqrt(differenceSumsSquared / len (delayValues[i])))

    colors = cm.rainbow ( np.linspace (0, 1, len (delayValues)))
    for i in range (0, len (delayValues)) : 
        n, bins, patches = plt.hist (delayValues[i], maxDelays[i]/100, color=colors[i], alpha=0.25)
        y = mlab.normpdf(bins, means[i], stdDevs[i])
        plt.plot(bins, y, color=colors[i])
 
    leg = plt.legend (resultFilenames)
    for l in leg.get_lines() :
        l.set_alpha(0.25)
    plt.xlabel ("Packet Delay (ns)")
    plt.ylabel ("Occurrencces")
    plt.savefig ("delay.pdf")


if __name__ == "__main__" :
    main()
