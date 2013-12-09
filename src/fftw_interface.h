#ifndef __FFTW_INTERFACE_H__
#define __FFTW_INTERFACE_H__

#include <fftw3.h>

#include "fft_interface.h"
#include "timer.h"

class FFTWInterface : public FFTInterface {
 public:
  FFTWInterface(const std::vector<std::complex<double>>& input,
                bool measure) : orig_input_(input), measure_(measure) {};

  bool Setup() {
    input_ = fftw_alloc_complex(orig_input_.size());
    if (input_ == nullptr) {
      return false;
    }
    output_ = fftw_alloc_complex(orig_input_.size());
    if (output_ == nullptr) {
      return false;
    }
    unsigned int flags = 0;
    if (measure_) {
      flags = FFTW_MEASURE;
    } else {
      flags = FFTW_ESTIMATE;
    }

    plan_ = fftw_plan_dft_1d(orig_input_.size(), input_, output_, FFTW_FORWARD,
                             flags);

    if (plan_ == nullptr) {
      return false;
    }
    memcpy(input_, orig_input_.data(),
           sizeof(fftw_complex) * orig_input_.size());
    return true;
  }

  bool RunTrial(std::vector<std::complex<double>>* output,
           double* running_time) {
    Timer timer; 
    fftw_execute(plan_);
    *running_time = timer.GetElapsedSeconds();

    output->resize(orig_input_.size());
    memcpy(output->data(), output_, sizeof(fftw_complex) * orig_input_.size());
    double normalization_factor = 1.0 / sqrt(orig_input_.size());
    for (size_t ii = 0; ii < orig_input_.size(); ++ii) {
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
    if (output_ != nullptr) {
      fftw_free(input_);
    }
  }

 private:
  const std::vector<std::complex<double>>& orig_input_;
  fftw_complex* input_;
  fftw_complex* output_;
  fftw_plan plan_;
  bool measure_;
};

#endif
