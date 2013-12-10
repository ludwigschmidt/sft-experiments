#ifndef __FFTW_INTERFACE_H__
#define __FFTW_INTERFACE_H__

#include <fftw3.h>

#include "fft_interface.h"
#include "timer.h"

class FFTWInterface : public FFTInterface {
 public:
  FFTWInterface(size_t n,
                bool measure) : n_(n), measure_(measure) {};

  bool Setup() {
    input_ = fftw_alloc_complex(n_);
    if (input_ == nullptr) {
      return false;
    }
    output_ = fftw_alloc_complex(n_);
    if (output_ == nullptr) {
      return false;
    }
    unsigned int flags = measure_ ? FFTW_MEASURE : FFTW_ESTIMATE;

    plan_ = fftw_plan_dft_1d(n_, input_, output_, FFTW_FORWARD, flags);

    if (plan_ == nullptr) {
      return false;
    }
    return true;
  }

  bool RunTrial(const std::vector<std::complex<double>>& input,
                std::vector<std::complex<double>>* output,
                double* running_time) {
    if (input.size() != n_) {
      return false;
    }
    memcpy(input_, input.data(), sizeof(fftw_complex) * n_);

    Timer timer; 
    fftw_execute(plan_);
    *running_time = timer.GetElapsedSeconds();

    output->resize(n_);
    memcpy(output->data(), output_, sizeof(fftw_complex) * n_);
    double normalization_factor = 1.0 / sqrt(n_);
    for (size_t ii = 0; ii < n_; ++ii) {
      (*output)[ii] *= normalization_factor;
    }
    return true;
  }

  ~FFTWInterface() {
    if (plan_ != nullptr) {
      fftw_destroy_plan(plan_);
    }
    if (output_ != nullptr) {
      fftw_free(output_);
    }
    if (input_ != nullptr) {
      fftw_free(input_);
    }
  }

 private:
  size_t n_;
  fftw_complex* input_;
  fftw_complex* output_;
  fftw_plan plan_;
  bool measure_;
};

#endif
