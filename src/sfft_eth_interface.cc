#include "sfft_eth_interface.h"

#include <cstring>

#include "timer.h"

bool SFFTETHInterface::Setup() {
  input_ = (complex_t*) sfft_malloc(sizeof(complex_t) * n_);
  if (input_ == nullptr) {
    return false;
  }

  int fftw_optimization = 0;
  if (measure_) {
    fftw_optimization = FFTW_MEASURE;
  } else {
    fftw_optimization = FFTW_ESTIMATE;
  }

  sfft_version version = SFFT_VERSION_1;
  if (version_ == Version::SFFT_1) {
    version = SFFT_VERSION_1;
  } else if (version_ == Version::SFFT_2) {
    version = SFFT_VERSION_2;
  } else if (version_ == Version::SFFT_3) {
    version = SFFT_VERSION_3;
  } else {
    return false;
  }

  plan_ = sfft_make_plan(n_, k_, version, fftw_optimization);

  if (plan_ == nullptr) {
    return false;
  }
  return true;
}

bool SFFTETHInterface::RunTrial(const std::vector<std::complex<double>>& input,
                                std::vector<std::complex<double>>* output,
                                double* running_time) {
  if (input.size() != n_) {
    return false;
  }
  memcpy(input_, input.data(), sizeof(complex_t) * n_);

  double rescaling = sqrt(n_); 
  for (size_t ii = 0; ii < n_; ++ii) {
    input_[ii] *= rescaling;
  }

  output_.clear();

  Timer timer; 
  sfft_exec(plan_, input_, &output_);
  *running_time = timer.GetElapsedSeconds();

  output->resize(n_);
  for (auto kv : output_) {
    (*output)[kv.first].real(creal(kv.second));
    (*output)[kv.first].imag(cimag(kv.second));
  }
  return true;
}

SFFTETHInterface::~SFFTETHInterface() {
  if (plan_ != nullptr) {
    sfft_free_plan(plan_);
  }
  if (input_ != nullptr) {
    sfft_free(input_);
  }
}
