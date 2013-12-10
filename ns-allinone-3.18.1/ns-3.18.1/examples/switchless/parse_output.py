#!/usr/bin/python

import sys

def usage () :
    print "Usage: " + sys.argv[0] + " [output file]"

def parseCmdArgs () :
    if len(sys.argv[1:]) != 1 :
        print "Invalid number of arguents"
        sys.exit(1)
    else :
        return sys.argv[1]

def main () :
    # Parse cmd line params
    outputFilename = parseCmdArgs()
    
    # Open output file
    outputFile = open(outputFilename, 'r')

    # Results
    latency = None
    sizeDelayTuples = []

    # Helper variables
    inRXBlock = False
    currentPacketSize = None

    # Loop over lines in file
    for line in outputFile :
        # Look for RX blocks
        if not inRXBlock :
            if line[:4] == "Node" and line[len(line)-4:-2] == "RX" :
                inRXBlock = True
        else :
            # Looking for parameters of interest in RX block
            # and save them
            if "    Packet Size: " in line :
                currentPacketSize = int(line[17:-7])
            elif "    RXTime: " in line :
                latency = float(line[12:-3])
            elif "    Delay: " in line :
                delay = float(line[11:-3])
                sizeDelayTuples.append((currentPacketSize, delay))
                inRXBlock = False 
            
    # Print output
    print "Latency: " + str(latency)
    print "(Packet Size, Delay) Values:"
    for sizeDelayTuple in sizeDelayTuples :
        print str(sizeDelayTuple)

if __name__ == "__main__":
    main()
