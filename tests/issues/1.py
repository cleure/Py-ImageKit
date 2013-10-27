
"""
* @date     2013-10-27
* @issue    Possible segfault found in Curves.from_bezier() when invalid data
*           is passed to it.
*
"""

import os, sys, random
from imagekit import *

NUM_SAMPLES = 1000
NUM_BEZIER_POINTS = 500

def main():
    curve = [[random.random(), random.random()] for i in range(NUM_BEZIER_POINTS)]
    points = Curves.from_bezier(NUM_SAMPLES, curve)

if __name__ == '__main__':
    try:
        main()
    except: pass
