#ifndef __OUTPUT_WRITER_H__
#define __OUTPUT_WRITER_H__

#include <complex>
#include <iostream>
#include <string>
#include <vector>

#include "result_helpers.h"

class OutputWriter {
 public:
  static std::string CollapseCommand(int argc, char** argv);

  OutputWriter(std::ostream* out, double l0_epsilon) : out_(out),
      delete_ostream_(false), l0_epsilon_(l0_epsilon) {}

  OutputWriter(const std::string& filename, double l0_epsilon);

  ~OutputWriter();
  
  bool WritePrelude(const std::string& command);
  bool WritePrelude(int argc, char** argv);

  bool WriteInputResult(const std::string& input_name,
                        const std::vector<std::complex<double>>& input_data,
                        const std::vector<std::complex<double>>& ref_output,
                        double reference_time,
                        const std::vector<RunResult>& results,
                        bool is_last);

  bool WriteEnd();

 private:
  std::ostream* out_;
  bool delete_ostream_;
  double l0_epsilon_;

  void WriteSignalStatistics(const SignalStatistics& stats, size_t indent);
};


#endif
