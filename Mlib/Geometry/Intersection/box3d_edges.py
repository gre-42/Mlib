#!/usr/bin/env python3

import numpy as np

c = set()

for i0 in range(2):
    for j0 in range(2):
        for k0 in range(2):
            for i1 in range(2):
                for j1 in range(2):
                    for k1 in range(2):
                        e0 = (i0, j0, k0)
                        e1 = (i1, j1, k1)
                        if np.count_nonzero(np.array(e1) - np.array(e0)) != 1:
                            continue
                        c |= {(min(e0, e1), max(e0, e1))}

print('\n'.join(f'  {e[0]} {e[1]}' for e in sorted(c)))
