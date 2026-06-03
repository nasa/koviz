#! /usr/bin/env /home/kvetter/.local/python311/bin/python3.11

import pyarrow as pa
import pyarrow.parquet as pq

nruns = 10
nsteps = 20

time = []
run = []
position = []
velocity = []

for t in range(nsteps):
    for run_id in range(1, nruns + 1):

        time.append(float(t))
        run.append(run_id)

        position.append(run_id * 1000.0 + t)
        velocity.append(run_id * 10.0 + t)

table = pa.table({
    "time": time,
    "run": run,
    "position": position,
    "velocity": velocity,
})

metadata = {
    b"time": b"s",
    b"run": b"--",
    b"position": b"m",
    b"velocity": b"m/s",
}

table = table.replace_schema_metadata(metadata)

pq.write_table(
    table,
    "monte_test.parquet",
)
