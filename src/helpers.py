import math

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
  
