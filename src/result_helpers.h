#ifndef __RESULT_HELPERS_H__
#define __RESULT_HELPERS_H__

#include <algorithm>
#include <cmath>
#include <complex>
#include <ostream>
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
  SignalStatistics topk_error_statistics;
  SignalStatistics output_statistics;
};

void ComputeSignalStatistics(const std::vector<std::complex<double>>& signal,
                             double l0_epsilon,
                             SignalStatistics* stats);

void ComputeErrorStatistics(const std::vector<std::complex<double>>& output,
    const std::vector<std::complex<double>>& reference_output,
    double l0_epsilon, SignalStatistics* stats);

void ComputeTopKErrorStatistics(const std::vector<std::complex<double>>& output,
    const std::vector<std::complex<double>>& reference_output,
    double l0_epsilon, int k, SignalStatistics* stats);

void WriteStatisticsJSONToStream(const SignalStatistics& stats,
                                 size_t indent,
                                 std::ostream* out);

void ComputeBestKTermRepresentation(const std::vector<std::complex<double>>& x,
                                    size_t k,
                                    std::vector<std::complex<double>>* x_k);

#endif
