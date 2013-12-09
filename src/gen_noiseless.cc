// Generates a signal with uniformly random support. Each supported value has a
// uniformly random phase and absolute value 1.

#include <cmath>
#include <complex>
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <random>
#include <vector>

#include <boost/program_options.hpp>

namespace po = boost::program_options;

using std::complex;
using std::cout;
using std::endl;
using std::string;
using std::vector;

typedef complex<double> dcomplex;

bool WriteOutput(const vector<dcomplex>& data, const string& dest) {
  if (dest.length() == 0) {
    for (size_t ii = 0; ii < data.size(); ++ii) {
      if (ii != 0) {
        cout << " ";
      }
      cout << data[ii];
    }
    cout << endl;
  } else {
    FILE* fout = fopen(dest.c_str(), "wb");
    if (fout == nullptr) {
      fprintf(stderr, "Error opening file \"%s\".", dest.c_str());
      return false;
    }
    size_t num_written = fwrite(data.data(), sizeof(dcomplex), data.size(),
                                fout);
    if (num_written != data.size()) {
      fprintf(stderr, "Error writing data: %lu elements written, expected %lu."
                      "\n", num_written, data.size());
      return false;
    }

    if (fclose(fout) != 0) {
      fprintf(stderr, "Error closing file \"%s\".", dest.c_str());
      return false;
    }
  }
  return true;
}

int main(int argc, char** argv) {
  size_t k;
  size_t n;
  string output_file;
  uint_fast32_t seed;

  po::options_description desc("Allowed options");
  desc.add_options()
      ("help", "Show help message.")
      ("k", po::value<size_t>(&k)->default_value(0), "Sparsity")
      ("n", po::value<size_t>(&n)->default_value(0),
          "Size of the signal to be generated.")
      ("output_file", po::value<string>(&output_file)->default_value(""),
          "Output file name (or \"\" for stdout). The default is \"\".")
      ("seed", po::value<size_t>(&seed)->default_value(0),
          "Seed for the PRNG.");
  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);

  if (vm.count("help")) {
    cout << desc << endl;
    return 0;
  }

  if (k > n) {
    fprintf(stderr, "k can be at most n.\n");
    return false;
  }

  std::mt19937 prng(seed);

  // Positions
  vector<int> indices(n);
  for (size_t ii = 0; ii < n; ++ii) {
    indices[ii] = ii;
  }
  std::shuffle(indices.begin(), indices.end(), prng);

  // Signal
  vector<dcomplex> signal(n, dcomplex(0, 0));

  std::uniform_real_distribution<> phase_distribution(0, 2 * M_PI);
  for (size_t ii = 0; ii < k; ++ii) {
    size_t pos = indices[ii];
    double phase = phase_distribution(prng);
    signal[pos].real(std::cos(phase));
    signal[pos].imag(std::sin(phase));
  }

  if (!WriteOutput(signal, output_file)) {
    fprintf(stderr, "Error while writing output.\n");
    return 1;
  }

  return 0;
}
