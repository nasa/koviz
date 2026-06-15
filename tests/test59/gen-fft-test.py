#!/usr/bin/env python3

import math

dt = 0.001      # 1000 Hz sample rate
duration = 10.0

with open("fft_test.csv", "w") as f:
    f.write("sys.exec.out.time {s},signal\n")

    n = int(duration / dt)

    for i in range(n):
        t = i * dt

        y = (
            1.00 * math.sin(2.0 * math.pi *  5.0 * t) +
            0.50 * math.sin(2.0 * math.pi * 20.0 * t) +
            0.25 * math.sin(2.0 * math.pi * 50.0 * t) +
            0.75 * math.sin(2.0 * math.pi * 10.37 * t)
        )

        f.write(f"{t},{y}\n")
