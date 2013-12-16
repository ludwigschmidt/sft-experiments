#ifndef __RESULT_HELPERS_H__
#define __RESULT_HELPERS_H__

#include <algorithm>
#include <cmath>
#include <complex>
#include <vector>

struct SignalStatistics {
  size_t l0;
  double l1;
  double l2;
  double linf;
};

struct RunResult {
  double time;
  SignalStatistics error_statistics;
  SignalStatistics output_statistics;
};

void ComputeSignalStatistics(const std::vector<std::complex<double>>& signal,
                             double l0_epsilon,
                             SignalStatistics* stats);

void ComputeErrorStatistics(const std::vector<std::complex<double>>& output,
    const std::vector<std::complex<double>>& reference_output,
    double l0_epsilon, SignalStatistics* stats);

#endif
