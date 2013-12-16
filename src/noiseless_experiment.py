import math
import os
import random
import sys

from collections import namedtuple

import matplotlib.pyplot as plt

from gen_noiseless import gen_noiseless
from helpers import build_time_stats
from run_experiment import run_experiment, extract_running_times, \
    num_l0_errors, write_index_file


def index_filename(basedir, n, k):
  return os.path.join(basedir,
                      'input_index_n_{}_k_{}.txt'.format(n, k))

def data_filename(basedir, n, k, instance):
  return os.path.join(basedir,
                      'input_n_{}_k_{}_instance_{}.bin'.format(n, k, instance))

def results_filename(basedir, algo, n, k):
  return os.path.join(basedir,
                      'results_{}_n_{}_k_{}.json'.format(algo, n, k))


tmpdir = 'tmpdir2'
num_instances = 10
num_trials = 10
exp1 = 16
exp2 = 18
k = 50
l0_eps = 0.2
percentile_interval = 95
random.seed(2314082)

algs = ['fftw', 'sfft3-eth', 'sfft2-eth', 'sfft2-mit']
#algs = ['sfft3-eth', 'sfft2-eth']
results = {}
for alg in algs:
  results[alg] = {}

for exp in range(exp1, exp2 + 1):
  n = int(math.pow(2, exp))
  for alg in algs:
    results[alg][n] = 0

  print 'n = {}'.format(n)
  print '  generating input data ...'
  input_filename = []
  for instance in range(1, num_instances + 1):
    print '    instance {}'.format(instance)
    dataf = data_filename(tmpdir, n, k, instance)
    gen_noiseless(n, k, dataf, random.randint(0, 2000000000))
    input_filename.append(dataf)

  print '  writing index file ...'
  indexf = index_filename(tmpdir, n, k)
  write_index_file(indexf, input_filename)

  for alg in algs:
    resultsf = results_filename(tmpdir, alg, n, k)
    print '  algorithm: {}'.format(alg)
    r = run_experiment(n, k, indexf, alg, l0_eps, num_trials, resultsf)
    times = extract_running_times(r)
    results[alg][n] = build_time_stats(times, percentile_interval)
    ne = num_l0_errors(r)
    if ne > 0:
      print '    {} L0-errors occurred.'.format(ne)


print results

for alg in algs:
  data = results[alg]
  xvals = sorted(data.keys())
  yvals = [data[x].average for x in xvals]
  yerr_lower = [data[x].percentile_low for x in xvals]
  yerr_upper = [data[x].percentile_high for x in xvals]
  plt.errorbar(xvals, yvals, yerr=[yerr_lower, yerr_upper], fmt='-x',
      label=alg)
plt.loglog(basex=2)
plt.xlabel('n')
plt.ylabel('time (s)')
plt.legend()
plt.show()
