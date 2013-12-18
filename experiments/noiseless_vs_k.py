import math
import os
import random
import sys

from collections import namedtuple

import matplotlib.pyplot as plt

from gen_input import gen_input
from helpers import make_data_point, write_data_points_to_file, \
    plot_data_points, Tee, input_file_cmds_filename, index_filename, \
    data_filename, results_filename, plot_time_data_filename, \
    plot_l0_error_data_filename, script_output_filename
from run_experiment import run_experiment, extract_running_times, \
    num_l0_errors, write_index_file, extract_l0_errors, load_results_file, \
    num_l0_correct

tmpdir = 'tmpdir'
keep_input_files = False
num_instances = 2
num_trials = 5
n = int(math.pow(2, 22))
#kvals = [50, 100, 200, 500, 1000, 2000, 4000]
kvals = [2000, 4000]
l0_eps = 0.5
time_percentile_low = 0
time_percentile_high = 95
l0_error_percentile_low = 0
l0_error_percentile_high = 95
random.seed(7524019)

sys.stdout = Tee(script_output_filename(tmpdir))

input_file_commands = []

algs = ['fftw', 'sfft3-eth', 'sfft2-eth', 'sfft2-mit', 'aafft']
#algs = ['sfft3-eth', 'sfft2-eth']

for k in kvals:
  print 'k = {}'.format(k)
  print '  generating input data ...'
  input_filename = []
  for instance in range(1, num_instances + 1):
    print '    instance {}'.format(instance)
    dataf = data_filename(tmpdir, n, k, instance)
    cmd = gen_input(n, k, dataf, random.randint(0, 2000000000))
    input_file_commands.append((dataf, cmd))
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
  if not keep_input_files:
    for f in input_filename:
      os.remove(f)

with open(input_file_cmds_filename(tmpdir), 'w') as f:
  for (name, cmd) in input_file_commands:
    f.write('{} {}\n'.format(name, cmd))


time_results = {}
l0_results = {}
success_results = {}

for alg in algs:
  time_results[alg] = {}
  l0_results[alg] = {}
  success_results[alg] = {}

for k in kvals:
  for alg in algs:
    r = load_results_file(results_filename(tmpdir, alg, n, k))
    times = extract_running_times(r)
    time_results[alg][k] = make_data_point(times, time_percentile_low,
                                           time_percentile_high)
    l0_errors = extract_l0_errors(r)
    l0_results[alg][k] = make_data_point(l0_errors, l0_error_percentile_low,
                                         l0_error_percentile_high)
    success_results[alg][k] = float(num_l0_correct(r)) \
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
                            'k', 'time')
  write_data_points_to_file(l0_results[alg],
                            plot_l0_error_data_filename(tmpdir, alg),
                            'k', 'l0_error')
# Matplotlib
plt.figure(1)
for alg in algs:
  plot_data_points(time_results[alg], plt, alg, '-x')
plt.loglog(basex=10)
plt.xlabel('k')
plt.ylabel('time (s)')
plt.legend()

plt.figure(2)
for alg in algs:
  plot_data_points(l0_results[alg], plt, alg, 'x')
plt.semilogx(basex=10)
plt.xlabel('k')
plt.ylabel('l0 error (l0-epsilon: {:e})'.format(l0_eps))
plt.legend()

plt.show()
