import pandas as pd
import matplotlib.pyplot as plt

df = pd.read_csv("coarse.csv")

plt.figure(figsize=(8, 5))

plt.plot(df["threads"], df["mops"], marker="o", label="CoarseMap")

# Frontera architecture boundaries
plt.axvline(28, linestyle="--", alpha=0.6, label="Socket boundary (28)")
plt.axvline(56, linestyle="--", alpha=0.6, label="Core count (56)")

plt.xlabel("Thread Count")
plt.ylabel("Throughput (Mops/s)")
plt.title("CoarseMap Throughput vs. Thread Count")
plt.xticks(df["threads"])

plt.grid(alpha=0.25)
plt.legend()
plt.tight_layout()

plt.savefig("coarse.png", dpi=300)
plt.show()
