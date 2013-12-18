#include "fft_wrapper.h"

#include "aafft_interface.h"
#include "fftw_interface.h"
#include "sfft_eth_interface.h"
#include "sfft_mit_interface.h"

bool FFTWrapper::ParseType(const std::string& str, Type* type) {
  string lower = boost::algorithm::to_lower_copy(str);
  if (lower == "aafft") {
    *type = Type::AAFFT;
  } else if (lower == "fftw") {
    *type = Type::FFTW;
  } else if (lower == "sfft1-eth") {
    *type = Type::SFFT1_ETH;
  } else if (lower == "sfft2-eth") {
    *type = Type::SFFT2_ETH;
  } else if (lower == "sfft2-mit") {
    *type = Type::SFFT2_MIT;
  } else if (lower == "sfft3-eth") {
    *type = Type::SFFT3_ETH;
  } else {
    return false;
  }
  return true;
}

bool FFTWrapper::Setup() {
  if (type_ == Type::AAFFT) {
    fft_.reset(new AAFFTInterface(n_, k_));
  } else if (type_ == Type::FFTW) {
    fft_.reset(new FFTWInterface(n_, true));
  } else if (type_ == Type::SFFT1_ETH) {
    fft_.reset(new SFFTETHInterface(n_, k_, SFFTETHInterface::Version::SFFT_1,
          false));
  } else if (type_ == Type::SFFT2_ETH) {
    fft_.reset(new SFFTETHInterface(n_, k_, SFFTETHInterface::Version::SFFT_2,
          false));
  } else if (type_ == Type::SFFT2_MIT) {
    fft_.reset(new SFFTMITInterface(n_, k_));
  } else if (type_ == Type::SFFT3_ETH) {
    fft_.reset(new SFFTETHInterface(n_, k_, SFFTETHInterface::Version::SFFT_3,
          false));
  } else {
    fprintf(stderr, "Unknown FFT type.\n");
    return false;
  }

  if (!fft_->Setup()) {
    fprintf(stderr, "Error setting up internal FFT implementation.\n");
    return false;
  }

  return true;
}

bool FFTWrapper::RunTrial(const std::vector<std::complex<double>>& input,
    std::vector<std::complex<double>>* output,
    double* time) {
  if (input.size() != n_) {
    fprintf(stderr, "Error, input size does not match n_: %lu vs %lu\n",
        input.size(), n_);
    return false;
  }
  if (!fft_->RunTrial(input, output, time)) {
    fprintf(stderr, "Error while running internal FFT implementation.");
    return false;
  }
  if (output->size() != input.size()) {
    fprintf(stderr, "Dimension of output produced by the interal FFT "
        "implementation does not match the input dimension: "
        "%lu vs %lu (output vs input).",
        output->size(),
        input.size());
    return false;
  }
  return true;
}
