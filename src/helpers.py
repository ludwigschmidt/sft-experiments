import math

from collections import namedtuple

import numpy as np

RunningTimeStatistics = namedtuple('RunningTimeStatistics', ['average',
                                                             'percentile_low',
                                                             'percentile_high'])

def percentile(values, percentile, round_index_up):
  index = percentile * len(values) / 100.0
  if (round_index_up):
    index = int(math.ceil(index))
  else:
    index = int(math.floor(index))
  if index == len(values):
    index = index - 1
  return sorted(values)[index]


def build_time_stats(times, percent):
  low = (100.0 - percent) / 2.0
  high = 100.0 - (100.0 - percent) / 2.0
  avg = np.mean(times)
  plow = percentile(times, low, False)
  phigh = percentile(times, high, True)
  return RunningTimeStatistics(average=avg, percentile_low=plow,
      percentile_high=phigh)
