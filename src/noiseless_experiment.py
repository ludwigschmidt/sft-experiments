import math
import os
import random
import sys

from collections import namedtuple

import matplotlib.pyplot as plt

from gen_input import gen_input
from helpers import make_data_point, write_data_points_to_file, plot_data_points
from run_experiment import run_experiment, extract_running_times, \
    num_l0_errors, write_index_file, extract_l0_errors, load_results_file, \
    num_l0_correct

class Tee(object):
  def __init__(self, name):
    self.logfile = open(name, 'w')
    self.stdout = sys.stdout
    sys.stdout = self
  def __del__(self):
    sys.stdout = self.stdout
    self.logfile.close()
  def write(self, data):
    self.logfile.write(data)
    self.logfile.flush()
    self.stdout.write(data)


def input_file_cmds_filename(basedir):
  return os.path.join(basedir, 'input_file_commands.txt')

def index_filename(basedir, n, k):
  return os.path.join(basedir,
                      'input_index_n_{}_k_{}.txt'.format(n, k))

def data_filename(basedir, n, k, instance):
  return os.path.join(basedir,
                      'input_n_{}_k_{}_instance_{}.bin'.format(n, k, instance))

def results_filename(basedir, algo, n, k):
  return os.path.join(basedir,
                      'results_{}_n_{}_k_{}.json'.format(algo, n, k))

def plot_time_data_filename(basedir, algo):
  return os.path.join(basedir,
                      'plot_time_results_{}.txt'.format(algo))

def plot_l0_error_data_filename(basedir, algo):
  return os.path.join(basedir,
                      'plot_l0_error_results_{}.txt'.format(algo))

def script_output_filename(basedir):
  return os.path.join(basedir, 'script_output.txt')

tmpdir = 'tmpdir'
keep_input_files = False
num_instances = 2
num_trials = 5
exp1 = 14
#exp2 = 23
exp2 = 17
k = 50
l0_eps = 0.5
time_percentile_low = 0
time_percentile_high = 95
l0_error_percentile_low = 0
l0_error_percentile_high = 95
random.seed(7524019)

sys.stdout = Tee(script_output_filename(tmpdir))

input_file_commands = []

#algs = ['fftw', 'sfft3-eth', 'sfft2-eth', 'sfft2-mit', 'aafft']
algs = ['sfft3-eth', 'sfft2-eth']
nvals = [int(math.pow(2, exp)) for exp in range(exp1, exp2 + 1)]

for n in nvals:
  print 'n = {}'.format(n)
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
