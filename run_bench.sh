#!/bin/bash

set -eu

GITREV=$(git rev-parse --short HEAD)

function eval_cmd() {
	eval numactl --cpunodebind 1 --membind 1 $@
}

time=$(date "+%Y%m%d_%H%M%S")
cmake --build build-1.9.2 -j
cmake --build build-1.9.3-hexl -j
eval_cmd "build-1.9.2/bin/benchmark/lib-benchmark 2>&1 | cat > bench_normal_${GITREV}_$time.log"
eval_cmd "build-1.9.3-hexl/bin/benchmark/lib-benchmark 2>&1 | cat > bench_hexl_${GITREV}_$time.log"
