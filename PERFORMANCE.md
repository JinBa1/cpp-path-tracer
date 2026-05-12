# Performance Benchmark

## Methodology

- **Metric**: Wall-clock render time (render phase only, excludes scene parsing, BVH construction, PPM write)
- **Runs**: 3 per configuration, median reported
- **Baseline**: Single-threaded at -O2 (same binary as multi-threaded runs, `--threads 1`)
- **Thread counts**: 1, 2, 4, 8, 16, 32
- **Scene**: path_scene.json (1200x800, 20 SPP, BVH enabled, path tracing with reflection)
- **Tiling**: 32x32 pixel blocks, pre-allocated framebuffer

## Hardware & Software

| Item | Value |
|------|-------|
| CPU | 13th Gen Intel Core i9-13900HX |
| Cores | 32 (8P + 16E + 8 hyperthreads) |
| Compiler | GCC 13.3.0 |
| Flags | -O2 -march=native |
| OS | Linux x86_64 |
| Commit | 91ffd43 |

## Scaling Results

| Threads | Median Time (s) | Speedup vs 1T | Parallel Efficiency |
|--------:|----------------:|---------------:|--------------------:|
| 1       | 37.35           | 1.00x          | --                  |
| 2       | 20.16           | 1.85x          | 92%                 |
| 4       | 8.96            | 4.16x          | 104%                |
| 8       | 4.74            | 7.87x          | 98%                 |
| 16      | 3.28            | 11.38x         | 71%                 |
| 32      | 2.38            | 15.69x         | 49%                 |

### Notes

- **4T superlinear** (104%): working set fits entirely in L2 cache at this thread count, reducing miss rates compared to the single-threaded baseline.
- **8T sweet spot** (98%): near-perfect efficiency across 8 P-cores. This is the best practical configuration for this CPU.
- **16T and 32T diminishing returns**: E-cores have lower per-thread throughput, and hyperthreads share execution units. Scaling continues but efficiency drops.

## Reproduce

```bash
bash scripts/benchmark.sh
```

Custom configuration:

```bash
BENCH_SCENE=../data/jsons/benchmark_scene.json BENCH_THREADS="1 4 8" bash scripts/benchmark.sh
```
