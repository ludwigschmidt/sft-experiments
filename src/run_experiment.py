from collections import namedtuple
import json
import subprocess

SignalStatistics = namedtuple('SignalStatistics', ['l0', 'l1', 'l2', 'linf'])
ExperimentResults = namedtuple('ExperimentResults',
                               ['command', 'results'])
InputResults = namedtuple('InputResults', ['input_stats', 'reference_time',
                                           'reference_output_stats', 'results'])
RunResult = namedtuple('RunResult', ['running_time', 'error_stats',
                                     'output_stats'])


def write_index_file(index_filename, input_filenames):
  with open(index_filename, 'w') as f:
    for fname in input_filenames:
      f.write(fname)
      f.write('\n')


def extract_stats(stats):
  l0 = int(stats['l0'])
  l1 = float(stats['l1'])
  l2 = float(stats['l2'])
  linf = float(stats['linf'])
  return SignalStatistics(l0=l0, l1=l1, l2=l2, linf=linf)


def extract_input_results(obj):
  istats = extract_stats(obj['input_stats'])
  reftime = float(obj['reference_time'])
  refostats = extract_stats(obj['reference_output_stats'])
  rresults = []
  for run in obj['results']:
    time = run['running_time']
    estats = extract_stats(run['error_stats'])
    ostats = extract_stats(run['output_stats'])
    rresults.append(RunResult(running_time=time, error_stats=estats,
                              output_stats=ostats))
  return InputResults(input_stats=istats, reference_time=reftime,
                      reference_output_stats=refostats, results=rresults)


def parse_results_json(obj):
  cmd = obj['command']
  input_results = {}
  for infile in obj['results'].keys():
    input_results[infile] = extract_input_results(obj['results'][infile])
  return ExperimentResults(command=cmd, results=input_results)


def extract_running_times(experiment_results):
  res = []
  for input_results in experiment_results.results.itervalues():
    for run in input_results.results:
      res.append(run.running_time)
  return res


def extract_l2_errors(experiment_results):
  res = []
  for input_results in experiment_results.results.itervalues():
    for run in input_results.results:
      res.append(run.error_stats.l2)
  return res


def extract_l0_errors(experiment_results):
  res = []
  for input_results in experiment_results.results.itervalues():
    for run in input_results.results:
      res.append(run.error_stats.l0)
  return res


def num_l0_errors(experiment_results):
  n = 0;
  for input_results in experiment_results.results.itervalues():
    for run in input_results.results:
      n += run.error_stats.l0
  return n


def num_l0_correct(experiment_results):
  n = 0;
  for input_results in experiment_results.results.itervalues():
    for run in input_results.results:
      if run.error_stats.l0 == 0:
        n += 1
  return n


def load_results_file(filename):
  with open(filename, 'r') as f:
    return parse_results_json(json.load(f))


def run_experiment(n, k, input_index, algorithm, l0_epsilon, num_trials,
                   output_file):
  cmd = ['./run_experiment']
  cmd.extend(['--n', str(n)])
  cmd.extend(['--k', str(k)])
  cmd.extend(['--input_index', input_index])
  cmd.extend(['--algorithm', algorithm])
  cmd.extend(['--l0_epsilon', str(l0_epsilon)])
  cmd.extend(['--num_trials', str(num_trials)])
  if (len(output_file) > 0):
    cmd.extend(['--output_file', output_file])
  subprocess.call(cmd, stdin=None, stderr=subprocess.STDOUT)
  return load_results_file(output_file)
