#include "output_writer.h"

#include <fstream>
#include <sstream>

using std::complex;
using std::endl;
using std::ostream;
using std::ostringstream;
using std::scientific;
using std::string;
using std::vector;

typedef complex<double> dcomplex;

OutputWriter::OutputWriter(const std::string& filename, double l0_epsilon) {
  l0_epsilon_ = l0_epsilon;

  if (filename.empty()) {
    out_ = &std::cout;
    delete_ostream_ = false;
  } else {
    out_= new std::ofstream(filename);
    delete_ostream_ = true;
  }
}

string OutputWriter::CollapseCommand(int argc, char** argv) {
  ostringstream command;
  for (int ii = 0; ii < argc; ++ii) {
    if (ii != 0) {
      command << " ";
    }
    command << argv[ii];
  }
  return command.str();
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
  ostream& oref = *out_;
  string indentation = "";
  for (size_t ii = 0; ii < indent; ++ii) {
    indentation += " ";
  }

  oref << indentation << "\"l0\": " << stats.l0 << "," << endl;
  oref << indentation << "\"l1\": " << scientific << stats.l1 << "," << endl;
  oref << indentation << "\"l2\": " << scientific << stats.l2 << "," << endl;
  oref << indentation << "\"linf\": " << scientific << stats.linf << endl;
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
