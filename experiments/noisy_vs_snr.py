import math
import os
import random
import sys

from collections import namedtuple

import matplotlib.pyplot as plt

from gen_input import gen_input, snr_db_to_noise_variance
from helpers import make_data_point, write_data_points_to_file, \
    plot_data_points, Tee, data_stats_filename_snr, index_filename_snr, \
    data_filename_snr, results_filename_snr, plot_time_data_filename, \
    plot_topk_l1_error_per_entry_data_filename, script_output_filename, \
    plot_relative_l2_l2_error_data_filename, db_to_ratio, ratio_to_db
from run_experiment import run_experiment, extract_running_times, \
    write_index_file, extract_topk_l1_errors, load_results_file, \
    compute_relative_l2_l2_errors

tmpdir = '/media/ludo/backup750/sfft_experiments4/noisy_vs_snr'
#num_instances = 2
num_instances = 10
#num_trials = 2
num_trials = 10
n = int(math.pow(2, 22))
k = 50
#snr_db_vals = [-20, -10, -7, -3, 0, 3, 7, 10, 20, 30, 40, 50, 60, 120]
snr_db_vals = [-20, -10, 0, 10, 20, 30, 40, 50, 60, 70]
#snr_db_vals = [10, 20]
l0_eps = 1e-8
relative_l2_l2_error_threshold = 1.3
time_percentile_low = 0
time_percentile_high = 95
topk_l1_error_percentile_low = 0
topk_l1_error_percentile_high = 95
relative_l2_l2_error_percentile_low = 0
relative_l2_l2_error_percentile_high = 95
random.seed(14389295)
plot = False

sys.stdout = Tee(script_output_filename(tmpdir))

#algs = ['fftw', 'sfft2-mit', 'sfft1-mit', 'aafft', 'sfft1-eth', 'sfft2-eth']
algs = ['fftw', 'sfft2-mit', 'sfft1-mit', 'aafft']
#algs = ['sfft2-mit']

for snr in snr_db_vals:
  print 'snr = {} db  ({:.6e})'.format(snr, db_to_ratio(snr))
  print '  generating input data ...'
  input_filename = []
  for instance in range(1, num_instances + 1):
    print '    instance {}'.format(instance)
    dataf = data_filename_snr(tmpdir, n, k, snr, instance)
    gen_input(n, k, dataf, seed=random.randint(0, 2000000000),
        stats_file=data_stats_filename_snr(tmpdir, n, k, snr, instance),
        noise_variance=snr_db_to_noise_variance(snr, n, k),
        randomize_phase=True)
    input_filename.append(dataf)
  print '  writing index file ...'
  indexf = index_filename_snr(tmpdir, n, k, snr)
  write_index_file(indexf, input_filename)
  for alg in algs:
    resultsf = results_filename_snr(tmpdir, alg, n, k, snr)
    print '  algorithm: {}'.format(alg)
    r = run_experiment(n, k, indexf, alg, l0_eps, num_trials,
        seed=random.randint(0, 2000000000), output_file=resultsf)
    rel_l2_l2 = compute_relative_l2_l2_errors(r)
    ne = 0
    for err in rel_l2_l2:
      if err > relative_l2_l2_error_threshold:
        ne += 1
    if ne > 0:
      print '    {} large l2/l2-errors occurred (threshold: {}).'.format(ne,
          relative_l2_l2_error_threshold)

  for f in input_filename:
    os.remove(f)

time_results = {}
topk_l1_results = {}
relative_l2_l2_results = {}

for alg in algs:
  time_results[alg] = {}
  topk_l1_results[alg] = {}
  relative_l2_l2_results[alg] = {}

for snr in snr_db_vals:
  for alg in algs:
    r = load_results_file(results_filename_snr(tmpdir, alg, n, k, snr))
    times = extract_running_times(r)
    time_results[alg][snr] = make_data_point(times, time_percentile_low,
                                           time_percentile_high)
    topk_l1_errors = extract_topk_l1_errors(r)
    for i in range(len(topk_l1_errors)):
      topk_l1_errors[i] = float(topk_l1_errors[i]) / k
    topk_l1_results[alg][snr] = make_data_point(topk_l1_errors,
                                                topk_l1_error_percentile_low,
                                                topk_l1_error_percentile_high)

    l2_l2_errors = compute_relative_l2_l2_errors(r)
    relative_l2_l2_results[alg][snr] = make_data_point(l2_l2_errors,
        relative_l2_l2_error_percentile_high,
        relative_l2_l2_error_percentile_low)

print '\n'
print 'Time results:\n'
print time_results
print '\n\naverage top-k l1-error results:\n'
print topk_l1_results
print '\n\nRelative l2/l2 errors:\n'
print relative_l2_l2_results

# pgfplot files
for alg in algs:
  write_data_points_to_file(time_results[alg],
                            plot_time_data_filename(tmpdir, alg),
                            'snr', 'time')
  write_data_points_to_file(topk_l1_results[alg],
                            plot_topk_l1_error_per_entry_data_filename(tmpdir,
                                alg),
                            'snr', 'topk_l1_error_per_entry')
  write_data_points_to_file(relative_l2_l2_results[alg],
                            plot_relative_l2_l2_error_data_filename(tmpdir,
                                alg), 'snr', 'relative_l2_l2_error')
# Matplotlib
if plot:
  plt.figure(1)
  for alg in algs:
    plot_data_points(time_results[alg], plt, alg, '-x')
  plt.loglog(basex=10)
  plt.xlabel('SNR')
  plt.ylabel('time (s)')
  plt.legend()

  plt.figure(2)
  for alg in algs:
    plot_data_points(topk_l1_results[alg], plt, alg, '-x')
  plt.semilogx(basex=10)
  plt.xlabel('SNR')
  plt.ylabel('top-k l1 error per entry')
  plt.legend()

  plt.figure(3)
  for alg in algs:
    plot_data_points(relative_l2_l2_results[alg], plt, alg, '-x')
  plt.semilogx(basex=10)
  plt.xlabel('SNR')
  plt.ylabel('relative l2/l2 error')
  plt.legend()

  plt.show()
