#include "sfft_mit_interface.h"

#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstring>

#include "sfft_mit/computefourier.h"
#include "sfft_mit/parameters.h"
#include "sfft_mit/utils.h"

#include "timer.h"

bool SFFTMITInterface::InternalSetup() {
  input_ = (complex_t*) malloc(sizeof(complex_t) * n_);
  if (input_ == nullptr) {
    return false;
  }

  double Bcst_loc = 1.0;
  double Bcst_est = 1.0;
  double Comb_cst = 2.0;
  loops_loc_ = 4;
  loops_est_ = 16;
  loops_thresh_ = 3;
  Comb_loops_ = 1;
  double tolerance_loc = 1.0e-8;
  double tolerance_est = 1.0e-8;

  loops_loc_ = -1;

  if (k_ == 50) {
    get_expermient_vs_N_parameters(n_, true, Bcst_loc, Bcst_est, Comb_cst,
        loops_loc_, loops_est_, loops_thresh_, Comb_loops_, tolerance_loc,
        tolerance_est);
  } else if (n_ == 4194304) {
    get_expermient_vs_K_parameters(k_, true, Bcst_loc, Bcst_est, Comb_cst,
        loops_loc_, loops_est_, loops_thresh_, Comb_loops_, tolerance_loc,
        tolerance_est);
  } else {
    fprintf(stderr, "Set of parameters n, k for which SFFT_MIT parameters are "
                    "not known.\n");
    return false;
  }

  if (loops_loc_ == -1) {
    fprintf(stderr, "Set of parameters n, k for which SFFT_MIT parameters are "
        "not known.\n");
  }

  real_t BB_loc = (unsigned) (Bcst_loc * sqrt((double) n_ * k_ / log2(n_)));
  real_t BB_est = (unsigned) (Bcst_est * sqrt((double) n_ * k_ / log2(n_)));

  double lobefrac_loc = 0.5 / (BB_loc);
  double lobefrac_est = 0.5 / (BB_est);
  int b_loc = int(1.2 * 1.1 * ((double) n_ / BB_loc));
  int b_est = int(1.4 * 1.1 * ((double) n_ / BB_est));

  B_loc_ = sfft_mit::floor_to_pow2(BB_loc);
  B_thresh_ = 2 * k_;
  B_est_ = sfft_mit::floor_to_pow2(BB_est);

  W_Comb_ = sfft_mit::floor_to_pow2(Comb_cst * n_ / B_loc_);

  assert(B_thresh_ < W_Comb_);

  int w_loc;
  complex_t* filtert = make_dolphchebyshev_t(lobefrac_loc, tolerance_loc,
      w_loc);
  filter_ = make_multiple_t(filtert, w_loc, n_, b_loc);

  int w_est;
  complex_t* filtert_est = make_dolphchebyshev_t(lobefrac_est, tolerance_est,
      w_est);
  filter_est_ = make_multiple_t(filtert_est, w_est, n_, b_est);

  return true;
}

bool SFFTMITInterface::Setup() {
  return InternalSetup();
}

bool SFFTMITInterface::RunTrial(const std::vector<std::complex<double>>& input,
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

  /*printf("\nn = %lu  B_est_ = %d  B_thresh_ = %d  B_loc_ = %d  W_Comb_ = %d  "
         "Comb_loops_ = %d  loops_thresh_ = %d  loops_loc_ = %d  "
         "loops_loc_ + loops_est_ = %d\n\n", n_, B_est_, B_thresh_, B_loc_,
         W_Comb_, Comb_loops_, loops_thresh_, loops_loc_,
         loops_loc_ + loops_est_);
  printf("filter_: sizet = %d  time[10] = (%lf,%lf)  freq[10] = (%lf,%lf)\n",
      filter_.sizet, creal(filter_.time[10]), cimag(filter_.time[10]),
      creal(filter_.freq[10]), creal(filter_.freq[10]));
  printf("filter_est_: sizet = %d  time[10] = (%lf,%lf)  freq[10] = (%lf,%lf)"
      "\n", filter_est_.sizet, creal(filter_est_.time[10]),
      cimag(filter_est_.time[10]), creal(filter_est_.freq[10]),
      creal(filter_est_.freq[10]));*/

  Timer timer; 
  output_ = outer_loop(input_, n_, filter_, filter_est_, B_est_, B_thresh_,
      B_loc_, W_Comb_, Comb_loops_, loops_thresh_, loops_loc_,
      loops_loc_ + loops_est_);
  *running_time = timer.GetElapsedSeconds();

  output->resize(n_);
  output->assign(n_, std::complex<double>(0.0, 0.0));
  for (auto kv : output_) {
    (*output)[kv.first].real(creal(kv.second));
    (*output)[kv.first].imag(cimag(kv.second));
  }
  
  return true;
}

SFFTMITInterface::~SFFTMITInterface() {
  InternalTearDown();
}

void SFFTMITInterface::InternalTearDown() {
  if (input_ != nullptr) {
    free(input_);
  }
  if (filter_.freq != nullptr) {
    free(filter_.freq);
  }
  if (filter_.time != nullptr) {
    free(filter_.time);
  }
  if (filter_est_.freq != nullptr) {
    free(filter_est_.freq);
  }
  if (filter_est_.time != nullptr) {
    free(filter_est_.time);
  }
}
