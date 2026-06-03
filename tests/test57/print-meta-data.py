#! /usr/bin/env /home/kvetter/.local/python311/bin/python3.11

import pyarrow.parquet as pq
pf = pq.ParquetFile("monte_test.parquet")
print(pf.metadata.metadata.keys())

for k,v in pf.metadata.metadata.items():
    print(k, v[:100])
