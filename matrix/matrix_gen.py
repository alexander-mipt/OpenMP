#!/usr/bin/env python

import numpy as np
import sys

def main():
    print(sys.argv[2])
    matrix = np.random.rand(int(sys.argv[1]),int(sys.argv[2]))
    print(matrix)

if __name__ == '__main__':
    main()