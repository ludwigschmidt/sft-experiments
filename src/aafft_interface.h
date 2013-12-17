#ifndef __AAFFT_INTERFACE_H__
#define __AAFFT_INTERFACE_H__

#include <complex>
#include <cstdio>
#include <iostream>
#include <vector>

// Need these using statements in order to compile AAFFT
using std::complex;
using std::cout;
using std::endl;
using std::vector;

// Need these include statements in order to compile AAFFT
#include <fftw3.h>

#include "aafft/AAarith_prog.h"
#include "aafft/AADFT_engine.h"
#include "aafft/AAmulti_array.h"
#include "aafft/AAparameters.h"
#include "aafft/AAfourier1D.h"

#include "fft_interface.h"
#include "timer.h"

class AAFFTInterface : public FFTInterface {
 public:
  AAFFTInterface(size_t n, size_t k) : n_(n), k_(k) {};

  bool Setup();

  bool RunTrial(const std::vector<std::complex<double>>& input,
                std::vector<std::complex<double>>* output,
                double* running_time);

  ~AAFFTInterface() {}

 private:
  size_t n_;
  size_t k_;
  std::vector<Rep_Term> output_;
  Parameters params_;

  static std::vector<std::complex<double>> input_;
  static std::complex<double> GetInput(unsigned int ii, int) {
    return input_[ii];
  }

  bool InternalSetup();
  bool SetImportantParameters();
};

std::vector<std::complex<double>> AAFFTInterface::input_;

bool AAFFTInterface::InternalSetup() {
  bool is_power_of_2 = (n_ & (n_ - 1)) == 0;
  if (!is_power_of_2) {
    fprintf(stderr, "Currently only power-of-2 signal dimensions are "
            "supported.\n");
    return false;
  }

  input_.resize(n_);

  // Parameter setup

  //////////////////////////////////////////////////
  // Parameters that are common across all use cases
  
  // Signal size
  params_.set_Signal_Size(n_);

  // Working_Rep is generally set to Num_Rep, which is k_
  params_.set_Num_Rep_Terms(k_);
  params_.set_Working_Rep_Terms(k_);

  // always 0 if n_ is a power of 2
  params_.set_Exhaustive_Most_Sig_Bits(0);

  // always 8
  params_.set_Roots_Coef(8);

  // use AAFFT 0.9
  params_.set_Naive_Bulk_Cutoff(1);

  // use AAFFT 0.9
  params_.set_Naive_Coef_Est_Cutoff(1);

  // documentation says 10 is sufficient most of the time. example uses 7.
  params_.set_Num_Fast_Bulk_Samp_Taylor_Terms(7);

  // always 8
  params_.set_FFCE_Roots_Coef(8);

  // documentation says 10 is sufficient most of the time. example uses 7.
  params_.set_Num_Fast_Freq_Coefnt_Est_Taylor_Terms(7);

  //////////////////////////////////////////////////
  // Important parameters
  if (!SetImportantParameters()) {
    return false;
  }

  return true;
}

bool AAFFTInterface::SetImportantParameters() {
  // These parameter settings follow Haitham's choices for the SFFT SODA paper.

  // Corresponding to command line variables
  int I, S, M, E;

  if (k_ == 50) {
    I = 5;
    E = 5;
    S = 128;
    M = 5;
  } else if (k_ == 100) {
    I = 6;
    E = 5;
    S = 128;
    M = 5;
  } else if (k_ == 200) {
    I = 6;
    E = 6;
    S = 200;
    M = 6;
  } else if (k_ == 500) {
    I = 7;
    E = 6;
    S = 440;
    M = 6;
  } else if (k_ == 1000) {
    I = 8;
    E = 7;
    S = 640;
    M = 7;
  } else if (k_ == 2000) {
    I = 8;
    E = 8;
    S = 1100;
    M = 9;
  } else if (k_ == 4000) {
    I = 8;
    E = 8;
    S = 2000;
    M = 11;
  } else {
    fprintf(stderr, "Unsupported parameters\n");
    return false;
  }

  // important
  // Haitham: -I
  params_.set_Num_FreqID_CoefEst_Iterations(I);

  // important
  // Haitham: -S
  params_.set_Max_KShattering_Sample_Points(S);
  params_.set_Num_KShattering_Sample_Points(S);

  // important
  // Haitham: -S
  params_.set_Max_FCE_Sample_Points(S);
  params_.set_Num_FCE_Sample_Points(S);

  // important
  // Haitham: -M
  params_.set_Norm_Estimation_Max(M);
  params_.set_Norm_Estimation_Num(M);

  // important
  // Haitham: -M
  params_.set_Max_FCE_Medians(M);
  params_.set_Num_FCE_Medians(M);

  // important
  // Haitham: -E
  params_.set_FFCE_Iterations(E);

  return true;
}

bool AAFFTInterface::Setup() {
  return InternalSetup();
}

bool AAFFTInterface::RunTrial(const std::vector<std::complex<double>>& input,
                              std::vector<std::complex<double>>* output,
                              double* running_time) {
  if (input.size() != n_) {
    return false;
  }
  input_ = input;

  output_.clear();

  DFT_engine tmp_dft_engine(0, 0);

  Timer timer;
  Fast_DFT(params_, GetInput, output_, tmp_dft_engine);
  *running_time = timer.GetElapsedSeconds();

  output->resize(n_);
  output->assign(n_, std::complex<double>(0.0, 0.0));
  for (Rep_Term term : output_) {
    (*output)[term.frequency] = term.coefficient;
  }
  
  return true;
}

#endif
