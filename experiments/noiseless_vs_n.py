import math
import os
import random
import sys

from collections import namedtuple

import matplotlib.pyplot as plt

from gen_input import gen_input
from helpers import make_data_point, write_data_points_to_file, \
    plot_data_points, Tee, data_stats_filename, index_filename, \
    data_filename, results_filename, plot_time_data_filename, \
    plot_l0_error_data_filename, script_output_filename
from run_experiment import run_experiment, extract_running_times, \
    num_l0_errors, write_index_file, extract_l0_errors, load_results_file, \
    num_l0_correct

tmpdir = '/media/ludo/external_linux/sfft_experiments/tmpdir1'
num_instances = 10
num_trials = 10
exp1 = 14
exp2 = 24
#exp2 = 17
k = 50
l0_eps = 0.5
time_percentile_low = 0
time_percentile_high = 95
l0_error_percentile_low = 0
l0_error_percentile_high = 95
random.seed(7524019)
plot = False

sys.stdout = Tee(script_output_filename(tmpdir))

algs = ['fftw', 'sfft3-eth', 'sfft2-eth', 'sfft2-mit', 'sfft1-eth', 'sfft1-mit', 'aafft']
#algs = ['sfft3-eth', 'sfft2-eth']
#algs = ['sfft3-eth']
nvals = [int(math.pow(2, exp)) for exp in range(exp1, exp2 + 1)]

for n in nvals:
  print 'n = {}'.format(n)
  print '  generating input data ...'
  input_filename = []
  for instance in range(1, num_instances + 1):
    print '    instance {}'.format(instance)
    dataf = data_filename(tmpdir, n, k, instance)
    gen_input(n, k, dataf, random.randint(0, 2000000000),
        stats_file=data_stats_filename(tmpdir, n, k, instance))
    input_filename.append(dataf)
  print '  writing index file ...'
  indexf = index_filename(tmpdir, n, k)
  write_index_file(indexf, input_filename)
  for alg in algs:
    resultsf = results_filename(tmpdir, alg, n, k)
    print '  algorithm: {}'.format(alg)
    r = run_experiment(n, k, indexf, alg, l0_eps, num_trials, resultsf)
    ne = num_l0_errors(r)
    if ne > 0:
      print '    {} L0-errors occurred.'.format(ne)
  for f in input_filename:
    os.remove(f)

time_results = {}
l0_results = {}
success_results = {}

for alg in algs:
  time_results[alg] = {}
  l0_results[alg] = {}
  success_results[alg] = {}

for n in nvals:
  for alg in algs:
    r = load_results_file(results_filename(tmpdir, alg, n, k))
    times = extract_running_times(r)
    time_results[alg][n] = make_data_point(times, time_percentile_low,
                                           time_percentile_high)
    l0_errors = extract_l0_errors(r)
    l0_results[alg][n] = make_data_point(l0_errors, l0_error_percentile_low,
                                         l0_error_percentile_high)
    success_results[alg][n] = float(num_l0_correct(r)) \
                                  / (num_instances * num_trials)

print '\n'
print 'Time results:\n'
print time_results
print '\n\nl0-error results:\n'
print l0_results
print '\n\nSuccess results:\n'
print success_results

# pgfplot files
for alg in algs:
  write_data_points_to_file(time_results[alg],
                            plot_time_data_filename(tmpdir, alg),
                            'n', 'time')
  write_data_points_to_file(l0_results[alg],
                            plot_l0_error_data_filename(tmpdir, alg),
                            'n', 'l0_error')
# Matplotlib
if plot:
  plt.figure(1)
  for alg in algs:
    plot_data_points(time_results[alg], plt, alg, '-x')
  plt.loglog(basex=2)
  plt.xlabel('n')
  plt.ylabel('time (s)')
  plt.legend()

  plt.figure(2)
  for alg in algs:
    plot_data_points(l0_results[alg], plt, alg, 'x')
  plt.semilogx(basex=2)
  plt.xlabel('n')
  plt.ylabel('l0 error (l0-epsilon: {:e})'.format(l0_eps))
  plt.legend()

  plt.show()
