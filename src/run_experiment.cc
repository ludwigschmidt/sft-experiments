#include <complex>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include <boost/program_options.hpp>

#include "fft_wrapper.h"
#include "fftw_helper.h"
#include "output_writer.h"
#include "result_helpers.h"

namespace po = boost::program_options;

using std::cin;
using std::complex;
using std::cout;
using std::endl;
using std::getline;
using std::ifstream;
using std::istream;
using std::round;
using std::string;
using std::vector;

typedef complex<double> dcomplex;

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

bool GetFileLines(const string& filename, vector<string>* lines) {
  ifstream infile(filename);
  if (!infile.good()) {
    fprintf(stderr, "Could not open file %s.\n", filename.c_str());
    return false;
  }
  string line;
  while (getline(infile, line)) {
    lines->push_back(line);
  }
  return true;
}

void RoundReal(vector<dcomplex>* data) {
  for (size_t ii = 0; ii < data->size(); ++ii) {
    (*data)[ii].real(round((*data)[ii].real()));
    (*data)[ii].imag(0.0);
  }
}


int main(int argc, char** argv) {
  string algorithm;
  string input_file;
  string input_index;
  size_t k;
  double l0_epsilon;
  size_t n;
  size_t num_trials;
  size_t num_warmup_runs;
  bool rounded_real_output;
  string output_file;
  size_t seed;

  po::options_description desc("Allowed options");
  desc.add_options()
      ("algorithm", po::value<string>(&algorithm)->default_value(""),
          "FFT algorithm to benchmark. Options: fftw, sfft3, sfft3-eth.")
      ("help", "Show help message.")
      ("input_file", po::value<string>(&input_file)->default_value(""),
          "Input file name for binary input (or \"\" for text data from "
          "stdin). The default is \"\".")
      ("input_index", po::value<string>(&input_index)->default_value(""),
          "File name of the input index file, which contains one input file "
          "name per line. Empty string if no index file should be used. The "
          "default is \"\".")
      ("k", po::value<size_t>(&k)->default_value(0), "Sparsity")
      ("l0_epsilon", po::value<double>(&l0_epsilon)->default_value(1e-8),
          "Threshold for l0-norm computation.")
      ("n", po::value<size_t>(&n)->default_value(0),
          "Number of elements in the input.")
      ("num_trials", po::value<size_t>(&num_trials)->default_value(1),
          "Number of trials.")
      ("num_warmup_runs", po::value<size_t>(&num_warmup_runs)->default_value(1),
          "Number of warm-up runs.")
      ("rounded_real_output", "Keep only the rounded real part of the output.")
      ("output_file", po::value<string>(&output_file)->default_value(""),
          "Output file name (or \"\" for stdout). The default is \"\".")
      ("seed", po::value<size_t>(&seed)->default_value(3492858),
          "Seed for the standard C PRNG.");
  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);

  if (vm.count("help")) {
    cout << desc << endl;
    return 0;
  }

  srand(seed);

  rounded_real_output = vm.count("rounded_real_output");

  FFTWrapper::Type fft_type;
  if (!FFTWrapper::ParseType(algorithm, &fft_type)) {
    fprintf(stderr, "Unknown algorithm type \"%s\".", algorithm.c_str());
    return 1;
  }

  vector<string> input_file_names;
  if (!input_index.empty()) {
    if (!input_file.empty()) {
      fprintf(stderr, "Error: cannot use input index and input file at the "
          "same time.");
      return 1;
    }
    if (!GetFileLines(input_index, &input_file_names)) {
      fprintf(stderr, "Error while reading the index file.");
      return 1;
    }
  } else {
    input_file_names.push_back(input_file);
  }

  FFTWrapper fft(n, k, fft_type);
  if (!fft.Setup()) {
    fprintf(stderr, "Could not set up algorithm.\n"); 
    return 1;
  }

  OutputWriter owriter(output_file, k, l0_epsilon);
  if (!owriter.WritePrelude(argc, argv)) {
    fprintf(stderr, "Could not write output.\n");
    return 1;
  }

  for (size_t jj = 0; jj < input_file_names.size(); ++jj) {
    const string& in_file_name = input_file_names[jj];
    vector<dcomplex> input_data;
    double reference_time;
    vector<dcomplex> reference_output;

    if (!ReadInput(in_file_name, n, &input_data)) {
      fprintf(stderr, "Could not read input file %s.\n", in_file_name.c_str());
      return 1;
    }

    if (!ApplyFFTW(input_data, true, true, false, &reference_time,
                   &reference_output)) {
      fprintf(stderr, "Could not compute reference output.\n");
      return 1;
    }
    if (rounded_real_output) {
      RoundReal(&reference_output);
    }

    RunResult current_result;
    vector<dcomplex> output;

    // Warm-up runs
    for (size_t ii = 0; ii < num_warmup_runs; ++ii) {
      fft.RunTrial(input_data, &output, &current_result.time);
    }

    vector<RunResult> results;
    for (size_t ii = 0; ii < num_trials; ++ii) {
      fft.RunTrial(input_data, &output, &current_result.time);

      if (rounded_real_output) {
        RoundReal(&output);
      }

      ComputeErrorStatistics(output, reference_output, l0_epsilon,
          &(current_result.error_statistics));
      ComputeSignalStatistics(output, l0_epsilon,
                              &(current_result.output_statistics));
      results.push_back(current_result);
    }

    if (!owriter.WriteInputResult(in_file_name, input_data, reference_output,
        reference_time, results, (jj == input_file_names.size() - 1))) {
      fprintf(stderr, "Could not write output.\n");
      return 1;
    }
  }
 
  if (!owriter.WriteEnd()) {
    fprintf(stderr, "Could not write output.\n");
    return 1;
  }

  return 0;
}
