#! /usr/bin/env python3

# AI Generated this

import h5py
import numpy as np

# Create generic time-history HDF5 for koviz testing
f = h5py.File("koviz_generic.h5", "w")

# /Storage/power
power_dtype = np.dtype([
    ("time", np.float64),
    ("battery_current", np.float64),
    ("solar_array_current", np.float64),
    ("mode", "S16")   # nonnumeric field
])

power = np.zeros(101, dtype=power_dtype)

for i in range(101):
    t = i * 0.1

    power[i]["time"] = t
    power[i]["battery_current"] = 5.0 + np.sin(t)
    power[i]["solar_array_current"] = 10.0 + np.cos(t)

    if t < 5.0:
        power[i]["mode"] = b"SUNLIGHT"
    else:
        power[i]["mode"] = b"ECLIPSE"

f.create_dataset("/Storage/power", data=power)

# /Storage/dynamics
dyn_dtype = np.dtype([
    ("time", np.float64),
    ("mass", np.float64),
    ("position_x", np.float64),
    ("position_y", np.float64),
    ("velocity_x", np.float64),
    ("velocity_y", np.float64),
])

dyn = np.zeros(101, dtype=dyn_dtype)

for i in range(101):
    t = i * 0.1

    dyn[i]["time"] = t
    dyn[i]["mass"] = 1000.0 - 0.1 * t

    dyn[i]["position_x"] = t
    dyn[i]["position_y"] = t * t

    dyn[i]["velocity_x"] = 1.0
    dyn[i]["velocity_y"] = 2.0 * t

f.create_dataset("/Storage/dynamics", data=dyn)

f.close()

print("Wrote koviz_generic.h5")
