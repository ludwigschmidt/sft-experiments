#ifndef __FFTW_HELPER_H__
#define __FFTW_HELPER_H__

#include <complex>
#include <cstring>
#include <vector>

#include <fftw3.h>

#include "timer.h"

bool ApplyFFTW(const std::vector<std::complex<double>>& input,
               bool normalize,
               bool forward,
               bool measure,
               double* computation_time,
               std::vector<std::complex<double>>* output) {
  fftw_complex* data = fftw_alloc_complex(input.size());
  if (data == nullptr) {
    return false;
  }

  int sign = forward ? FFTW_FORWARD : FFTW_BACKWARD;
  unsigned int flags = measure ? FFTW_MEASURE : FFTW_ESTIMATE;

  fftw_plan plan = fftw_plan_dft_1d(input.size(), data, data, sign, flags);
  if (plan == nullptr) {
    fftw_free(data);
    return false;
  }

  memcpy(data, input.data(), sizeof(fftw_complex) * input.size());

  Timer timer; 
  fftw_execute(plan);
  *computation_time = timer.GetElapsedSeconds();

  output->resize(input.size());
  memcpy(output->data(), data, sizeof(fftw_complex) * input.size());

  if (normalize) {
    double normalization_factor = 1.0 / sqrt(input.size());
    for (size_t ii = 0; ii < input.size(); ++ii) {
      (*output)[ii] *= normalization_factor;
    }
  }

  fftw_destroy_plan(plan);
  fftw_free(data);

  return true;  
}

#endif
