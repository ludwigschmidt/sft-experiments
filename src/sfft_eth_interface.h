#ifndef __SFFT_ETH_INTERFACE_H__
#define __SFFT_ETH_INTERFACE_H__

#include "sfft_eth/sfft.h"

#include "fft_interface.h"

class SFFTETHInterface : public FFTInterface {
 public:
  enum class Version {
    SFFT_1,
    SFFT_2,
    SFFT_3
  };

  SFFTETHInterface(size_t n, size_t k, Version version, bool measure)
      : n_(n), k_(k), version_(version), measure_(measure) {};

  bool Setup();

  bool RunTrial(const std::vector<std::complex<double>>& input,
                std::vector<std::complex<double>>* output,
                double* running_time);

  ~SFFTETHInterface();

 private:
  size_t n_;
  size_t k_;
  Version version_;
  complex_t* input_;
  sfft_output output_;
  sfft_plan* plan_;
  bool measure_;
};

#endif
