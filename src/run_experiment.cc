#include <algorithm>
#include <cmath>
#include <complex>
#include <cstdio>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>

#include <boost/algorithm/string.hpp>
#include <boost/program_options.hpp>

#include "fft_interface.h"
#include "fftw_helper.h"
#include "fftw_interface.h"
#include "sfft_eth_interface.h"

namespace po = boost::program_options;

using std::abs;
using std::cin;
using std::complex;
using std::cout;
using std::endl;
using std::istream;
using std::max;
using std::ostringstream;
using std::string;
using std::unique_ptr;
using std::vector;

struct SignalStatistics {
  size_t l0;
  double l1;
  double l2;
  double linf;
};

struct RunResult {
  double time;
  SignalStatistics error_statistics;
};

typedef complex<double> dcomplex;

void ComputeSignalStatistics(const vector<dcomplex>& signal,
                             double l0_epsilon,
                             SignalStatistics* stats) {
  stats->l0 = 0;
  stats->l1 = 0.0;
  stats->l2 = 0.0;
  stats->linf = 0.0;

  double absval;
  for (size_t ii = 0; ii < signal.size(); ++ii) {
    absval = abs(signal[ii]);
    if (absval > l0_epsilon) {
      ++(stats->l0);
    }
    stats->l1 += absval;
    stats->l2 += absval * absval;
    stats->linf = max(stats->linf, absval);
  }

  stats->l2 = sqrt(stats->l2);
}

bool ReadComplexBinaryData(FILE* file, size_t n, vector<dcomplex>* data) {
  data->resize(n);
  size_t num_read = fread(data->data(), 2 * sizeof(double), n, file);
  if (num_read != n) {
    fprintf(stderr, "Read only %lu input elements, not %lu.\n", num_read, n);
    return false;
  }
  return true;
}

bool ReadComplexTextData(istream* input, size_t n, vector<dcomplex>* data) {
  data->resize(n);
  for (size_t ii = 0; ii < n; ++ii) {
    if (!((*input) >> (*data)[ii])) {
      fprintf(stderr, "Could not read element %lu from stdin.\n", ii);
      return false;
    }
  }
  return true; 
}

bool ReadInput(const string& src, size_t n, vector<dcomplex>* data) {
  if (src.length() == 0) {
    return ReadComplexTextData(&cin, n, data);
  } else {
    FILE* input = fopen(src.c_str(), "rb");
    if (input == nullptr) {
      fprintf(stderr, "Could not open file %s.\n", src.c_str());
      return false;
    }
    if (!ReadComplexBinaryData(input, n, data)) {
      fclose(input);
      return false;
    }
    // Check if file is empty
    uint8_t tmp;
    if (fread(&tmp, sizeof(uint8_t), 1, input) != 0) {
      fprintf(stderr, "Input not empty afer reading %lu complex numbers.\n", n);
      return false;
    }
    if (fclose(input) != 0) {
      fprintf(stderr, "Could not close input.\n");
    }
  }
  return true;
}

bool WriteOutput(int argc,
                 char** argv,
                 const vector<dcomplex>& input,
                 const vector<dcomplex>& reference_output,
                 double reference_time,
                 const vector<RunResult>& results,
                 const string& dest,
                 double l0_epsilon) {
  FILE* out;
  if (dest.length() == 0) {
    out = stdout;
  } else {
    out = fopen(dest.c_str(), "w");
    if (out == nullptr) {
      fprintf(stderr, "Could not open file \"%s\" for writing.", dest.c_str());
    }
  }

  ostringstream command;
  for (int ii = 0; ii < argc; ++ii) {
    if (ii != 0) {
      command << " ";
    }
    command << argv[ii];
  }
  string cmd_str = command.str();

  fprintf(out, "{\n");
  fprintf(out, "  \"command\": \"%s\",\n", cmd_str.c_str());

  SignalStatistics input_stats;
  ComputeSignalStatistics(input, l0_epsilon, &input_stats);
  fprintf(out, "  \"input_stats\": {\n");
  fprintf(out, "    \"l0\": %lu,\n", input_stats.l0);
  fprintf(out, "    \"l1\": %e,\n", input_stats.l1);
  fprintf(out, "    \"l2\": %e,\n", input_stats.l2);
  fprintf(out, "    \"linf\": %e\n", input_stats.linf);
  fprintf(out, "  },\n");

  fprintf(out, "  \"reference_time\": %e,\n", reference_time);
  SignalStatistics ref_output_stats;
  ComputeSignalStatistics(reference_output, l0_epsilon, &ref_output_stats);
  fprintf(out, "  \"reference_output_stats\": {\n");
  fprintf(out, "    \"l0\": %lu,\n", ref_output_stats.l0);
  fprintf(out, "    \"l1\": %e,\n", ref_output_stats.l1);
  fprintf(out, "    \"l2\": %e,\n", ref_output_stats.l2);
  fprintf(out, "    \"linf\": %e\n", ref_output_stats.linf);
  fprintf(out, "  },\n");

  fprintf(out, "  \"results\": [\n");
  for (size_t ii = 0; ii < results.size(); ++ii) {
    fprintf(out, "    {\n");
    fprintf(out, "      \"running_time\": %e,\n", results[ii].time);
    fprintf(out, "      \"error_stats\": {\n");
    fprintf(out, "        \"l0\": %lu,\n", results[ii].error_statistics.l0);
    fprintf(out, "        \"l1\": %e,\n", results[ii].error_statistics.l1);
    fprintf(out, "        \"l2\": %e,\n", results[ii].error_statistics.l2);
    fprintf(out, "        \"linf\": %e\n", results[ii].error_statistics.linf);
    fprintf(out, "      }\n");
    if (ii != results.size() - 1) {
      fprintf(out, "    },\n");
    } else {
      fprintf(out, "    }\n");
    }
  }
  fprintf(out, "  ]\n");
  fprintf(out, "}\n");

  if (fclose(out) != 0) {
    fprintf(stderr, "Error closing output file stream.");
    return false;
  }

  return true;
}

class FFT {
 public:
  enum class Type {
      FFTW,
      SFFT3,
      SFFT3_ETH,
  };

  static bool ParseType(const string& str, Type* type) {
    string lower = boost::algorithm::to_lower_copy(str);
    if (lower == "fftw") {
      *type = Type::FFTW;
    } else if (lower == "sfft3") {
      *type = Type::SFFT3;
    } else if (lower == "sfft3-eth") {
      *type = Type::SFFT3_ETH;
    } else {
      return false;
    }
    return true;
  }

  FFT(size_t n, size_t k, Type type) : n_(n), k_(k), type_(type) { };

  bool Setup() {
    if (type_ == Type::FFTW) {
      fft_.reset(new FFTWInterface(n_, true));
    } else if (type_ == Type::SFFT3_ETH) {
      fft_.reset(new SFFTETHInterface(n_, k_, SFFTETHInterface::Version::SFFT_3,
                                      false));
    }
    
    if (!fft_->Setup()) {
      fprintf(stderr, "Error setting up internal FFT implementation.");
      return false;
    }

    return true;
  }

  bool RunTrial(const vector<dcomplex>& input,
                vector<dcomplex>* output,
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

 private:
  size_t n_;
  size_t k_;
  Type type_;
  unique_ptr<FFTInterface> fft_;
};

void ComputeErrorStatistics(const vector<dcomplex>& output,
                            const vector<dcomplex>& reference_output,
                            double l0_epsilon,
                            RunResult* result) {
  vector<dcomplex> error(output.size());
  for (size_t ii = 0; ii < output.size(); ++ii) {
    error[ii] = reference_output[ii] - output[ii];
  }
  ComputeSignalStatistics(error, l0_epsilon, &(result->error_statistics));
}

int main(int argc, char** argv) {
  string algorithm;
  string input_file;
  size_t k;
  double l0_epsilon;
  size_t n;
  size_t num_trials;
  string output_file;

  po::options_description desc("Allowed options");
  desc.add_options()
      ("algorithm", po::value<string>(&algorithm)->default_value(""),
          "FFT algorithm to benchmark. Options: fftw, sfft3, sfft3-eth.")
      ("help", "Show help message.")
      ("input_file", po::value<string>(&input_file)->default_value(""),
          "Input file name for binary input (or \"\" for text data from "
          "stdin). The default is \"\".")
      ("k", po::value<size_t>(&k)->default_value(0), "Sparsity")
      ("l0_epsilon", po::value<double>(&l0_epsilon)->default_value(1e-8),
          "Threshold for l0-norm computation.")
      ("n", po::value<size_t>(&n)->default_value(0),
          "Number of elements in the input.")
      ("num_trials", po::value<size_t>(&num_trials)->default_value(1),
          "Number of trials.")
      ("output_file", po::value<string>(&output_file)->default_value(""),
          "Output file name (or \"\" for stdout). The default is \"\".");
  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);

  if (vm.count("help")) {
    cout << desc << endl;
    return 0;
  }

  FFT::Type fft_type;
  if (!FFT::ParseType(algorithm, &fft_type)) {
    fprintf(stderr, "Unknown algorithm type \"%s\".", algorithm.c_str());
    return 1;
  }
  vector<dcomplex> input_data;
  double reference_time;
  vector<dcomplex> reference_output;

  if (!ReadInput(input_file, n, &input_data)) {
    fprintf(stderr, "Could not read input.\n");
    return 1;
  }

  if (!ApplyFFTW(input_data, true, true, false, &reference_time,
                 &reference_output)) {
    fprintf(stderr, "Could not compute reference output.\n");
    return 1;
  }

  FFT fft(n, k, fft_type);

  if (!fft.Setup()) {
    fprintf(stderr, "Could not set up algorithm.\n"); 
    return 1;
  }

  // Warm-up run
  RunResult current_result;
  vector<dcomplex> output;
  fft.RunTrial(input_data, &output, &current_result.time);

  vector<RunResult> results;
  for (size_t ii = 0; ii < num_trials; ++ii) {
    fft.RunTrial(input_data, &output, &current_result.time);
    ComputeErrorStatistics(output, reference_output, l0_epsilon,
        &current_result);
    results.push_back(current_result);
  }

  WriteOutput(argc, argv, input_data, reference_output, reference_time, results,
      output_file, l0_epsilon);

  return 0;
}
