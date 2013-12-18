#include "helpers.h"

#include <sstream>

std::string CollapseCommand(int argc, char** argv) {
  std::ostringstream command;
  for (int ii = 0; ii < argc; ++ii) {
    if (ii != 0) {
      command << " ";
    }
    command << argv[ii];
  }
  return command.str();
}
