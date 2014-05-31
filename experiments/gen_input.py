import subprocess

import helpers

def snr_db_to_noise_variance(snr, n, k):
  return float(k) / float(2 * helpers.db_to_ratio(snr) * n)

def gen_input(n, k, output_file, seed, randomize_phase=False, stats_file='',
    noise_variance=-1):
  cmd = ['./gen_input']
  cmd.extend(['--n', str(n)])
  cmd.extend(['--k', str(k)])
  cmd.extend(['--output_file', output_file])
  cmd.extend(['--seed', str(seed)])
  if not randomize_phase:
    cmd.append('--skip_phase_randomization')
  if len(stats_file) > 0:
    cmd.extend(['--stats_file', stats_file])
  if noise_variance > 0:
    var_string = '{:.6e}'.format(noise_variance)
    cmd.extend(['--noise_variance', var_string])
  subprocess.check_output(cmd, stdin=None, stderr=subprocess.STDOUT)
