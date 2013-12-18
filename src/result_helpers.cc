#include "result_helpers.h"

#include <algorithm>
#include <cmath>
#include <map>

#include "helpers.h"

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

void WriteStatisticsJSONToStream(const SignalStatistics& stats,
                                 size_t indent,
                                 std::ostream* out) {
  std::ostream& oref = *out;
  std::string indentation = "";
  for (size_t ii = 0; ii < indent; ++ii) {
    indentation += " ";
  }

  oref << indentation << "\"l0\": " << stats.l0 << "," << std::endl;
  oref << indentation << "\"l1\": " << std::scientific << stats.l1 << ","
      << std::endl;
  oref << indentation << "\"l2\": " << std::scientific << stats.l2 << ","
      << std::endl;
  oref << indentation << "\"linf\": " << std::scientific << stats.linf
      << std::endl;
}

void ComputeBestKTermRepresentation(const std::vector<std::complex<double>>& x,
                                    size_t k,
                                    std::vector<std::complex<double>>* x_k) {
  size_t n = x.size();
  x_k->resize(n);
  x_k->assign(n, std::complex<double>(0.0, 0.0));

  std::vector<std::pair<double, size_t>> coeffs;
  coeffs.reserve(n);
  for (size_t ii = 0; ii < n; ++ii) {
    coeffs.push_back(std::make_pair(std::abs(x[ii]), ii));
  }
  std::sort(coeffs.begin(), coeffs.end());
  for (size_t ii = n - 1; ii >= n - k; --ii) {
    (*x_k)[coeffs[ii].second] = x[coeffs[ii].second];
  }
}
