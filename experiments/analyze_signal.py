import re
import sys

import numpy as np

import matplotlib.pyplot as plt

prog = re.compile('\((.*),(.*)\)')

def parse_complex(txt):
  match = prog.match(txt)
  if not match:
    print 'ERROR: could not parse complex number: ' + txt
    sys.exit(1)
  return complex(float(match.group(1)), float(match.group(2)))
  

txtdata = sys.stdin.readline()
parts = txtdata.split(' ')
data = [parse_complex(x) for x in parts]

n = len(data)
print 'n: ' + str(n)

mean = np.mean(data)
print 'mean: ' + str(mean)

var = np.var(data)
print 'var: ' + str(var)

plt.scatter([x.real for x in data], [x.imag for x in data])
plt.axis('equal')
plt.show()
