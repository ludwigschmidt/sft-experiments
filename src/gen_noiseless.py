import subprocess

def gen_noiseless(n, k, output_file, seed):
  cmd = ['./gen_noiseless']
  cmd.extend(['--n', str(n)])
  cmd.extend(['--k', str(k)])
  cmd.extend(['--output_file', output_file])
  cmd.extend(['--seed', str(seed)])
  subprocess.check_output(cmd, stdin=None, stderr=subprocess.STDOUT)
