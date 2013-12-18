#include "output_writer.h"

#include <fstream>
#include <sstream>

#include "helpers.h"

using std::complex;
using std::endl;
using std::ostream;
using std::ostringstream;
using std::scientific;
using std::string;
using std::vector;

typedef complex<double> dcomplex;

OutputWriter::OutputWriter(const std::string& filename,
                           size_t k,
                           double l0_epsilon) {
  l0_epsilon_ = l0_epsilon;
  k_ = k;

  if (filename.empty()) {
    out_ = &std::cout;
    delete_ostream_ = false;
  } else {
    out_= new std::ofstream(filename);
    delete_ostream_ = true;
  }
}

bool OutputWriter::WritePrelude(const string& command) {
  ostream& oref = *out_;
  if (!oref.good()) {
    return false;
  }
  oref << "{" << endl;
  oref << "  \"command\": \"" << command << "\"," << endl;
  oref << "  \"results\": {" << endl;
  return oref.good();
}

bool OutputWriter::WritePrelude(int argc, char** argv) {
  string tmp = CollapseCommand(argc, argv);
  return WritePrelude(tmp);
}

bool OutputWriter::WriteEnd() {
  ostream& oref = *out_;
  if (!oref.good()) {
    return false;
  }
  oref << "  }" << endl;
  oref << "}" << endl;
  return oref.good();
}

void OutputWriter::WriteSignalStatistics(const SignalStatistics& stats,
                                         size_t indent) {
  WriteStatisticsJSONToStream(stats, indent, out_);
}

bool OutputWriter::WriteInputResult(const string& input_name,
                                    const vector<dcomplex>& input_data,
                                    const vector<dcomplex>& ref_output,
                                    double reference_time,
                                    const std::vector<RunResult>& results,
                                    bool is_last) {
  SignalStatistics input_stats;
  ComputeSignalStatistics(input_data, l0_epsilon_, &input_stats);
  SignalStatistics ref_output_stats;
  ComputeSignalStatistics(ref_output, l0_epsilon_, &ref_output_stats);

  vector<dcomplex> best_k_term;
  ComputeBestKTermRepresentation(ref_output, k_, &best_k_term);
  SignalStatistics best_k_term_stats;
  ComputeSignalStatistics(best_k_term, l0_epsilon_, &best_k_term_stats);
  SignalStatistics best_k_term_error_stats;
  ComputeErrorStatistics(best_k_term, ref_output, l0_epsilon_,
      &best_k_term_error_stats);

  ostream& oref = *out_;
  if (!oref.good()) {
    return false;
  }
  oref << "    \"" << input_name << "\": {" << endl;
  oref << "      \"input_stats\": {" << endl;
  WriteSignalStatistics(input_stats, 8);
  oref << "      }," << endl;
  oref << "      \"reference_time\": " << scientific << reference_time << ","
       << endl;
  oref << "      \"reference_output_stats\": {" << endl;
  WriteSignalStatistics(ref_output_stats, 8);
  oref << "      }," << endl;
  oref << "      \"best_k_term_stats\": {" << endl;
  WriteSignalStatistics(best_k_term_stats, 8);
  oref << "      }," << endl;
  oref << "      \"best_k_term_error_stats\": {" << endl;
  WriteSignalStatistics(best_k_term_error_stats, 8);
  oref << "      }," << endl;
  oref << "      \"results\": [" << endl;
  for (size_t ii = 0; ii < results.size(); ++ii) {
    oref << "        {" << endl;
    oref << "          \"running_time\": " << scientific << results[ii].time
         << "," << endl;
    oref << "          \"error_stats\": {" << endl;
    WriteSignalStatistics(results[ii].error_statistics, 12);
    oref << "          }," << endl;
    oref << "          \"output_stats\": {" << endl;
    WriteSignalStatistics(results[ii].output_statistics, 12);
    oref << "          }" << endl;
    oref << "        }" << (ii != results.size() - 1 ? "," : "") << endl;
  }
  oref << "      ]\n" << endl;
  oref << "    }" << (is_last ? "" : ",") << endl;

  return oref.good();
}

OutputWriter::~OutputWriter() {
  if (delete_ostream_) {
    delete out_;
  }
}
