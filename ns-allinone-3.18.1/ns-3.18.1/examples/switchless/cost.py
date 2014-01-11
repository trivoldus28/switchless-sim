#/usr/bin/python

import math


SILICON_COST = 20
PORTEXPANDER_COST = 50

def switchlessCost(n):
  return n * (SILICON_COST + PORTEXPANDER_COST);


COMMODITY_SWITCH_COST = 7000
HIGH_END_SWITCH = 14000
TRANSCEIVER_COST = 200
FIBER50M_COST = 50
FIBER100M_COST = 100
FANOUT = 48

def hierarchyLowCost(n):
  if n <= FANOUT:
    return COMMODITY_SWITCH_COST;
  elif n <= FANOUT * FANOUT:
    ntor = math.ceil(float(n) / FANOUT);
    nagg = 1
    nlink = ntor
    ntransceiver = nlink * 2
    return ntor * COMMODITY_SWITCH_COST + nagg * HIGH_END_SWITCH + nlink * FIBER50M_COST \
             + ntransceiver * TRANSCEIVER_COST;
  else:
    ntor = math.ceil(float(n) / FANOUT);
    nagg = math.ceil(float(ntor) / FANOUT);
    ncore = 1
    nlink50 = ntor * nagg
    nlink100 = ncore * nagg
    ntransceiver = (nlink100 + nlink50) * 2
    return ntor * COMMODITY_SWITCH_COST + nagg * HIGH_END_SWITCH + ncore * HIGH_END_SWITCH \
             + nlink50 * FIBER50M_COST + nlink100 * FIBER100M_COST \
             + ntransceiver * TRANSCEIVER_COST;


def hierarchyBalancedCost(n):
  if n <= FANOUT:
    return COMMODITY_SWITCH_COST;
  elif n <= FANOUT * FANOUT:
    ntor = math.ceil(pow(n,.5));
    nagg = 1
    nlink = ntor
    ntransceiver = nlink * 2
    return ntor * COMMODITY_SWITCH_COST + nagg * HIGH_END_SWITCH + nlink * FIBER50M_COST \
             + ntransceiver * TRANSCEIVER_COST;
  else:
    ntor = math.ceil(pow(n,.66666));
    nagg = math.ceil(pow(n,.33333));
    ncore = 1
    nlink50 = ntor * nagg
    nlink100 = ncore * nagg
    ntransceiver = (nlink100 + nlink50) * 2
    return ntor * COMMODITY_SWITCH_COST + nagg * HIGH_END_SWITCH + ncore * HIGH_END_SWITCH \
             + nlink50 * FIBER50M_COST + nlink100 * FIBER100M_COST \
             + ntransceiver * TRANSCEIVER_COST;


def hierarchyBalancedReplicatedCost(n, n1=4, n2=4):
  if n <= FANOUT:
    return COMMODITY_SWITCH_COST;
  elif n <= FANOUT * FANOUT:
    ntor = math.ceil(pow(n,.5));
    nagg = n1
    nlink = ntor
    ntransceiver = nlink * 2
    return ntor * COMMODITY_SWITCH_COST + nagg * HIGH_END_SWITCH + nlink * FIBER50M_COST \
             + ntransceiver * TRANSCEIVER_COST;
  else:
    ntor = math.ceil(pow(n,.66666));
    nagg = math.ceil(pow(n,.33333)) * n1;
    ncore = n1 * n2
    nlink50 = ntor * nagg
    nlink100 = ncore * nagg
    ntransceiver = (nlink100 + nlink50) * 2
    return ntor * COMMODITY_SWITCH_COST + nagg * HIGH_END_SWITCH + ncore * HIGH_END_SWITCH \
             + nlink50 * FIBER50M_COST + nlink100 * FIBER100M_COST \
             + ntransceiver * TRANSCEIVER_COST;

def fatTreeCost(n):
  # N = k^3 / 4
  # k = 3root(N*4)
  k = math.ceil(pow(n*4, .33333333333));
  k = math.ceil(pow(n*4, .33333333333));
  npodswitch = pow(k, 2)
  ntopswitch = pow(k, 2) / 4
  nlink100 = npodswitch * ntopswitch
  ntransceiver = nlink100 * 2
  return (npodswitch + ntopswitch) * COMMODITY_SWITCH_COST + nlink100 * 100 + ntransceiver * TRANSCEIVER_COST
