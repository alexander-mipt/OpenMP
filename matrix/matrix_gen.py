#!/usr/bin/env python3

import numpy as np
import argparse

def main():


    parser = argparse.ArgumentParser(description='Generates random matrix MxN with numbers from min to max.')
    parser.add_argument('M', type=int, nargs=1,
                        help='num of rows')
    parser.add_argument('N', type=int, nargs=1,
                        help='num of cols')
    parser.add_argument('min', type=int, nargs='?', default=-100,
                        help='min num in matrix')
    parser.add_argument('max', type=int, nargs='?', default=100,
                        help='max num in matrix')
                        
    args = parser.parse_args()

    rows= args.M[0]
    cols= args.N[0]

    matrix = np.random.randint(low=args.min, high=args.max, size=( int(rows), int(cols) ))

    for row in matrix:
        for col in row:
            print("{:<6}".format(col), end='')
        print()

if __name__ == '__main__':
    main()