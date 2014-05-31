#include <cmath>
#include <complex>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <random>
#include <string>
#include <vector>

#include <boost/program_options.hpp>

#include "fftw_helper.h"
#include "helpers.h"
#include "result_helpers.h"

namespace po = boost::program_options;

using std::complex;
using std::cout;
using std::endl;
using std::log10;
using std::ofstream;
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

bool WriteStatsFile(int argc, char** argv,
                    const vector<dcomplex>& pure_spectrum,
                    const vector<dcomplex>& spectrum_noise,
                    const vector<dcomplex>& final_signal,
                    const string& stats_filename) {
  ofstream out(stats_filename);
  if (!out.good()) {
    cout << "Error opening file \"" << stats_filename << "\"." << endl;
    return false;
  }

  string cmd = CollapseCommand(argc, argv);

  SignalStatistics pure_spectrum_stats;
  SignalStatistics spectrum_noise_stats;
  SignalStatistics final_signal_stats;
  ComputeSignalStatistics(pure_spectrum, 0.0, &pure_spectrum_stats);
  ComputeSignalStatistics(spectrum_noise, 0.0, &spectrum_noise_stats);
  ComputeSignalStatistics(final_signal, 0.0, &final_signal_stats);
  double snr = pure_spectrum_stats.l2 / spectrum_noise_stats.l2;
  snr = snr * snr;

  out << "{" << endl;
  out << "  \"command\": \"" << cmd << "\"," << endl;
  out << "  \"pure_spectrum_stats\": {" << endl;
  WriteStatisticsJSONToStream(pure_spectrum_stats, 4, &out);
  out << "  }," << endl;
  out << "  \"spectrum_noise_stats\": {" << endl;
  WriteStatisticsJSONToStream(spectrum_noise_stats, 4, &out);
  out << "  }," << endl;
  out << "  \"final_signal_stats\": {" << endl;
  WriteStatisticsJSONToStream(final_signal_stats, 4, &out);
  out << "  }," << endl;
  out << "  \"snr\": " << std::scientific << snr << "," << endl;
  out << "  \"snr_db\": " << std::scientific << 10.0 * log10(snr) << endl;
  out << "}" << endl;

  return out.good();
}

int main(int argc, char** argv) {
  size_t k;
  size_t n;
  string output_file;
  uint_fast32_t seed;
  double noise_variance;
  string stats_file;

  po::options_description desc("Allowed options");
  desc.add_options()
      ("firstk", "Do not randomize spectrum support, take the k first indices.")
      ("help", "Show help message.")
      ("k", po::value<size_t>(&k)->default_value(0), "Sparsity")
      ("n", po::value<size_t>(&n)->default_value(0),
          "Size of the signal to be generated.")
      ("noise_variance",
          po::value<double>(&noise_variance)->default_value(-1.0),
          "This parameter specifies the noise variance for both the real and "
          "imaginary component. If negative, no noise is added.")
      ("output_file", po::value<string>(&output_file)->default_value(""),
          "Output file name (or \"\" for stdout). The default is \"\".")
      ("skip_phase_randomization", "Do not randomize the phase.")
      ("seed", po::value<size_t>(&seed)->default_value(0),
          "Seed for the PRNG.")
      ("skip_ifft", "Do not apply an inverse FFT on the generated spectrum.")
      ("skip_normalization", "Do not normalize the output from FFTW.")
      ("stats_file", po::value<string>(&stats_file)->default_value(""),
          "File for the signal statistics. If the parameters is \"\", no "
          "statistics file will be written.");
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
  if (!vm.count("firstk")) {
    std::shuffle(indices.begin(), indices.end(), prng);
  }

  // Spectrum
  vector<dcomplex> signal(n, dcomplex(0, 0));

  std::uniform_real_distribution<> phase_distribution(0, 2 * M_PI);
  for (size_t ii = 0; ii < k; ++ii) {
    size_t pos = indices[ii];
    if (vm.count("skip_phase_randomization")) {
      signal[pos].real(1.0);
      signal[pos].imag(0.0);
    } else {
      double phase = phase_distribution(prng);
      signal[pos].real(std::cos(phase));
      signal[pos].imag(std::sin(phase));
    }
  }

  vector<dcomplex> noise(n, dcomplex(0, 0));
  vector<dcomplex> final_signal(signal);
  std::normal_distribution<> noise_distribution(0, sqrt(noise_variance));
  if (noise_variance > 0.0) {
    for (size_t ii = 0; ii < n; ++ii) {
      noise[ii] = dcomplex(noise_distribution(prng), noise_distribution(prng));
      final_signal[ii] += noise[ii];
    }
  }

  if (!vm.count("skip_ifft")) {
    double tmp;
    ApplyFFTW(final_signal, !vm.count("skip_normalization"), false, false, &tmp,
        &final_signal);
  }

  if (!WriteOutput(final_signal, output_file)) {
    fprintf(stderr, "Error while writing output.\n");
    return 1;
  }

  if (!stats_file.empty()) {
    if (!WriteStatsFile(argc, argv, signal, noise, final_signal, stats_file)) {
      fprintf(stderr, "Error while writing stats file.\n");
      return 1;
    }
  }

  return 0;
}
