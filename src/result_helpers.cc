#include "result_helpers.h"

void ComputeSignalStatistics(const std::vector<std::complex<double>>& signal,
                             double l0_epsilon,
                             SignalStatistics* stats) {
  stats->l0 = 0;
  stats->l1 = 0.0;
  stats->l2 = 0.0;
  stats->linf = 0.0;

  double absval;
  for (size_t ii = 0; ii < signal.size(); ++ii) {
    absval = std::abs(signal[ii]);
    if (absval > l0_epsilon) {
      ++(stats->l0);
    }
    stats->l1 += absval;
    stats->l2 += absval * absval;
    stats->linf = std::max(stats->linf, absval);
  }

  stats->l2 = sqrt(stats->l2);
}

void ComputeErrorStatistics(const std::vector<std::complex<double>>& output,
    const std::vector<std::complex<double>>& reference_output,
    double l0_epsilon, SignalStatistics* stats) {
  std::vector<std::complex<double>> error(output.size());
  for (size_t ii = 0; ii < output.size(); ++ii) {
    error[ii] = reference_output[ii] - output[ii];
  }
  ComputeSignalStatistics(error, l0_epsilon, stats);
}
