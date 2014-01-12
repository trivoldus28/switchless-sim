#!/usr/bin/python

import sys
import matplotlib
import matplotlib.pyplot as plt
import numpy as np
import cost as costmodel

def plotall(maxn, show=True, scale='log', normalized=False):
	switchless = []
	hierarchicallow = []
	hierarchicalbalanced = []
	hierarchicalreplicated = []
	fattree = []

	for n in range(1,maxn):
		if normalized:
			switchless.append(costmodel.switchlessCost(n) / n)
			hierarchicallow.append(costmodel.hierarchyLowCost(n) / n)
			hierarchicalbalanced.append(costmodel.hierarchyBalancedCost(n) / n)
			hierarchicalreplicated.append(costmodel.hierarchyBalancedReplicatedCost(n) / n)
			fattree.append(costmodel.fatTreeCost(n) / n)
		else:
			switchless.append(costmodel.switchlessCost(n))
			hierarchicallow.append(costmodel.hierarchyLowCost(n))
			hierarchicalbalanced.append(costmodel.hierarchyBalancedCost(n))
			hierarchicalreplicated.append(costmodel.hierarchyBalancedReplicatedCost(n))
			fattree.append(costmodel.fatTreeCost(n))
	plt.clf()
	plt.plot(fattree, label='fattree')
	plt.plot(hierarchicalreplicated, label='hier_replicated')
	plt.plot(hierarchicalbalanced, label='hier_balanced')
	plt.plot(hierarchicallow, label='hier_low')
	plt.plot(switchless, label='switchless')

	if scale == 'log':
		legendloc = 4
	else:
		legendloc = 2
	plt.legend(('fattree', 'hier_replicated', 'hier_balanced', 'hier_low', 'switchless'), loc=legendloc)
	plt.yscale(scale)
	if show:
		plt.show()


if __name__ == "__main__":

	arg1 = sys.argv[1]
	if arg1 == 'predefined':
		for n in [100,500,1000,5000,10000,20000]:
			plotall(n, False, 'log')
			plt.savefig(`n` + '_log_' + 'totalcost.png')
			plotall(n, False, 'linear')
			plt.savefig(`n` + '_linear_' + 'totalcost.png')
			plotall(n, False, 'log', True)
			plt.savefig(`n` + '_log_' + 'permachinecost.png')
			plotall(n, False, 'linear', True)
			plt.savefig(`n` + '_linear_' + 'permachinecost.png')
	else:	
		maxn = int(sys.argv[1])
		plotall(maxn, True, 'log')
