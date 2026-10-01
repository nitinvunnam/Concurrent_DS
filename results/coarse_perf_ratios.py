import pandas as pd

df = pd.read_csv("coarse_perf.csv")

t1 = df[df["threads"] == 1].iloc[0]
t8 = df[df["threads"] == 8].iloc[0]

cycles_growth = t8["cycles_per_op"] / t1["cycles_per_op"]
l1_growth = t8["l1_misses_per_op"] / t1["l1_misses_per_op"]

print("cycles/op growth:", cycles_growth)
print("L1 misses/op growth:", l1_growth)
