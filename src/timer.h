#ifndef __TIMER_H__
#define __TIMER_H__

#include <cstdint>

#include <time.h>

class Timer {
 public:
  static bool IsSupported() {
    timespec ts;
    return (clock_gettime(CLOCK_MONOTONIC_RAW, &ts) == 0);
  }

  static uint64_t GetNanosecondTimestamp() {
    timespec ts;
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    uint64_t rv;
    rv = ts.tv_sec;
    rv *= 1000000000;
    rv += ts.tv_nsec;
    return rv;
  }
  
  Timer() {
    start = GetNanosecondTimestamp();
  }

  double GetElapsedSeconds() {
    uint64_t current = GetNanosecondTimestamp();
    return static_cast<double>(current - start) * 1e-9;
  }

 private:
  uint64_t start;
};

#endif
