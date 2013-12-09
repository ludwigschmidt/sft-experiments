#ifndef __FFT_INTERFACE_H__
#define __FFT_INTERFACE_H__

#include <complex>
#include <vector>

class FFTInterface {
 public:
  virtual bool Setup() = 0;
  virtual bool RunTrial(std::vector<std::complex<double>>* output,
                        double* running_time) = 0;
  virtual ~FFTInterface() {}
};


#endif
