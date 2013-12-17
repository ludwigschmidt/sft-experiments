import subprocess

def gen_input(n, k, output_file, seed, randomize_phase=False):
  cmd = ['./gen_input']
  cmd.extend(['--n', str(n)])
  cmd.extend(['--k', str(k)])
  cmd.extend(['--output_file', output_file])
  cmd.extend(['--seed', str(seed)])
  if not randomize_phase:
    cmd.append('--skip_phase_randomization')
  subprocess.check_output(cmd, stdin=None, stderr=subprocess.STDOUT)
  return ' '.join(cmd)
