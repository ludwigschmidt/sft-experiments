#ifndef __SFFT_MIT_INTERFACE_H__
#define __SFFT_MIT_INTERFACE_H__

#include <map>

#include "sfft_mit/fft.h"
#include "sfft_mit/filters.h"

#include "fft_interface.h"

class SFFTMITInterface : public FFTInterface {
 public:
  enum class Version {
    SFFT_1,
    SFFT_2,
  };

  SFFTMITInterface(size_t n, size_t k, Version version) : version_(version),
      n_(n), k_(k) {};

  bool Setup();

  bool RunTrial(const std::vector<std::complex<double>>& input,
                std::vector<std::complex<double>>* output,
                double* running_time);

  ~SFFTMITInterface();

 private:
  Version version_;
  size_t n_;
  size_t k_;
  complex_t* input_;
  std::map<int, complex_t> output_;

  int B_est_;
  int B_thresh_;
  int B_loc_;
  int W_Comb_;
  int Comb_loops_;
  int loops_thresh_;
  int loops_loc_;
  int loops_est_;
  Filter filter_;
  Filter filter_est_;

  bool InternalSetup();

  void InternalTearDown();
};

#endif
