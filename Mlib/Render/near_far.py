#!/usr/bin/env python3

import numpy as np
z_b = np.array([0.8, 0.9, 0.94, 0.99, 0.999, 0.99999])
zNear = -1
zFar = -10000

z_n = 2.0 * z_b - 1.0
z_e = 2.0 * zNear * zFar / (zFar + zNear - z_n * (zFar - zNear))
print(z_e)
col_b = (z_e - zNear) / (zFar - zNear)
print(col_b)
