from collections import namedtuple
import json
import subprocess

SignalStatistics = namedtuple('SignalStatistics', ['l0', 'l1', 'l2', 'linf'])
ExperimentResults = namedtuple('ExperimentResults',
                               ['command', 'input_stats', 'reference_time',
                                'reference_output_stats', 'results'])
RunResult = namedtuple('RunResult', ['running_time', 'error_stats'])

def extract_stats(stats):
  l0 = float(stats['l0'])
  l1 = float(stats['l1'])
  l2 = float(stats['l2'])
  linf = float(stats['linf'])
  return SignalStatistics(l0=l0, l1=l1, l2=l2, linf=linf)


def parse_results_json(obj):
  cmd = obj['command']
  istats = extract_stats(obj['input_stats'])
  reftime = float(obj['reference_time'])
  refostats = extract_stats(obj['reference_output_stats'])
  rresults = []
  for run in obj['results']:
    time = run['running_time']
    estats = extract_stats(run['error_stats'])
    rresults.append(RunResult(running_time=time, error_stats=estats))
  return ExperimentResults(command=cmd, input_stats=istats,
                           reference_time=reftime,
                           reference_output_stats=refostats,
                           results=rresults)


def compute_average_running_time(result):
  s = 0.0
  for run in result.results:
    s += run.running_time
  return s / len(result.results)


def check_l0_errors(result):
  for run in result.results:
    if run.error_stats.l0 > 0:
      return False
  return True


def run_experiment(n, k, input_file, algorithm, l0_epsilon, num_trials,
                   output_file):
  cmd = ['./run_experiment']
  cmd.extend(['--n', str(n)])
  cmd.extend(['--k', str(k)])
  cmd.extend(['--input_file', input_file])
  cmd.extend(['--algorithm', algorithm])
  cmd.extend(['--l0_epsilon', str(l0_epsilon)])
  cmd.extend(['--num_trials', str(num_trials)])
  if (len(output_file) > 0):
    cmd.extend(['--output_file', output_file])
  subprocess.call(cmd, stdin=None, stderr=subprocess.STDOUT)
  
  with open(output_file, 'r') as f:
    result = parse_results_json(json.load(f))
  return result
