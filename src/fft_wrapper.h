#ifndef __FFT_WRAPPER_H__
#define __FFT_WRAPPER_H__

#include <complex>
#include <memory>
#include <string>
#include <vector>

#include <boost/algorithm/string.hpp>

#include "fft_interface.h"

class FFTWrapper {
 public:
  enum class Type {
      AAFFT,
      FFTW,
      SFFT1_ETH,
      SFFT2_ETH,
      SFFT2_MIT,
      SFFT3_ETH,
  };

  static bool ParseType(const std::string& str, Type* type);

  FFTWrapper(size_t n, size_t k, Type type) : n_(n), k_(k), type_(type) { };

  bool Setup();

  bool RunTrial(const std::vector<std::complex<double>>& input,
                std::vector<std::complex<double>>* output,
                double* time);

 private:
  size_t n_;
  size_t k_;
  Type type_;
  std::unique_ptr<FFTInterface> fft_;
};


#endif
