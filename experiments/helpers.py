import math
import os
import sys

from collections import namedtuple

import numpy as np

DataPoint = namedtuple('DataPoint', ['average', 'error_plus', 'error_minus'])

def percentile(values, percentile, round_index_up):
  index = percentile * len(values) / 100.0
  if (round_index_up):
    index = int(math.ceil(index))
  else:
    index = int(math.floor(index))
  if index == len(values):
    index = index - 1
  return sorted(values)[index]


def make_data_point(values, percentile_low, percentile_high):
  avg = np.mean(values)
  low = percentile(values, percentile_low, False)
  high = percentile(values, percentile_high, True)
  return DataPoint(average=avg,
                   error_plus=(high - avg),
                   error_minus=(avg - low))


def write_data_points_to_file(points, filename, xlabel, ylabel):
  with open(filename, 'w') as f:
    f.write('{} {} error_plus error_minus\n'.format(xlabel,ylabel))
    xvals = sorted(points.keys())
    for x in xvals:
      f.write('{} {} {} {}\n'.format(x, points[x].average, points[x].error_plus,
                                     points[x].error_minus))


def plot_data_points(points, plt, label, fmt):
  xvals = sorted(points.keys())
  yvals = [points[x].average for x in xvals]
  yerr_minus = [points[x].error_minus for x in xvals]
  yerr_plus = [points[x].error_plus for x in xvals]
  plt.errorbar(xvals, yvals, yerr=[yerr_minus, yerr_plus], fmt=fmt,
               label=label)


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


def index_filename(basedir, n, k):
  return os.path.join(basedir,
                      'input_index_n_{}_k_{}.txt'.format(n, k))

def data_filename(basedir, n, k, instance):
  return os.path.join(basedir,
                      'input_n_{}_k_{}_instance_{}.bin'.format(n, k, instance))

def data_stats_filename(basedir, n, k, instance):
  return os.path.join(basedir,
                      'input_n_{}_k_{}_instance_{}_stats.txt'.format(n, k,
                                                                     instance))

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
