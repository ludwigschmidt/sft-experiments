# Ludwig Schmidt (ludwigschmidt2@gmail.com) 2013
#
# This makefile is based on http://make.paulandlesley.org/autodep.html .

CXX = g++
MEX = mex
CXXFLAGS = -Wall -Wextra -ggdb -g3 -std=c++11 -pedantic

SRCDIR = src
DEPDIR = .deps
OBJDIR = obj

SRCS = run_experiment.cc

.PHONY: clean archive

clean:
	rm -rf $(OBJDIR)
	rm -rf $(DEPDIR)
	rm -f run_experiment
	rm -f sfft_benchmark.tar.gz

archive:
	mkdir archive-tmp
	tar --transform='s,^\.,sfft_benchmark,' --exclude='.git' --exclude='archive-tmp' -czf archive-tmp/sfft_benchmark.tar.gz .
	mv archive-tmp/sfft_benchmark.tar.gz .
	rm -rf archive-tmp

RUN_EXPERIMENT_OBJS = run_experiment.o

# run_experiment executable
run_experiment: $(RUN_EXPERIMENT_OBJS:%=$(OBJDIR)/%)
	$(CXX) $(CXXFLAGS) -o $@ $^ -lboost_program_options -lfftw3 -lrt


$(OBJDIR)/%.o: $(SRCDIR)/%.cc
  # Create the directory the current target lives in.
	@mkdir -p $(@D)
  # Compile and generate a dependency file.
  # See http://gcc.gnu.org/onlinedocs/gcc/Preprocessor-Options.html .
	$(CXX) $(CXXFLAGS) -MMD -MP -c -o $@ $<
  # Move dependency file to dependency file directory.
  # Create the dependency file directory if necessary.
	@mkdir -p $(DEPDIR)
	@mv $(OBJDIR)/$*.d $(DEPDIR)/$*.d

# Include the generated dependency files.
# The command replaces each file name in SRCS with its dependency file.
# See http://www.gnu.org/software/make/manual/html_node/Substitution-Refs.html#Substitution-Refs for the GNU make details.
-include $(SRCS:%.cc=$(DEPDIR)/%.d)
