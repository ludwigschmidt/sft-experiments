import math
import os
import random
import sys

import matplotlib.pyplot as plt

from run_experiment import run_experiment, compute_average_running_time, \
    check_l0_errors
from gen_noiseless import gen_noiseless


def data_filename(basedir, n, k, instance):
  return os.path.join(basedir,
                      'input_n_{}_k_{}_instance_{}.bin'.format(n, k, instance))

def results_filename(basedir, algo, n, k, instance):
  return os.path.join(basedir,
                      'results_{}_n_{}_k_{}_instance_{}.json'.format(algo,
                                                                     n, k,
                                                                     instance))

tmpdir = 'tmpdir'
num_instances = 10
num_trials = 10
exp1 = 10
exp2 = 20
k = 100
l0_eps = 0.01
random.seed(2314082)

algs = ['fftw', 'sfft3-eth']
results = {}
for alg in algs:
  results[alg] = {}

for exp in range(exp1, exp2 + 1):
  n = int(math.pow(2, exp))
  for alg in algs:
    results[alg][n] = 0

  print 'n = {}'.format(n)
  for instance in range(1, num_instances + 1):
    print '  instance {}'.format(instance)
    dataf = data_filename(tmpdir, n, k, instance)
    
    print '    generating input data ...'
    gen_noiseless(n, k, dataf, random.randint(0, 2000000000))

    for alg in algs:
      resultsf = results_filename(tmpdir, alg, n, k, instance)
      print '    algo: {}'.format(alg)
      r = run_experiment(n, k, dataf, alg, l0_eps, num_trials, resultsf)
      results[alg][n] += compute_average_running_time(r)
      if not check_l0_errors(r):
        print 'L0-ERROR with algorithm {} on input file {}.'.format(alg, dataf)

  for alg in algs:
    results[alg][n] /= num_instances

print results

for alg in algs:
  data = results[alg]
  xvals = sorted(data.keys())
  yvals = [data[x] for x in xvals]
  plt.plot(xvals, yvals, label=alg)
plt.loglog(basex=2)
plt.xlabel('n')
plt.ylabel('time (s)')
plt.legend()
plt.show()
