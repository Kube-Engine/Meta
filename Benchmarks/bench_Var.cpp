/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Meta Signal benchmark
 */

#include <benchmark/benchmark.h>

#include <Kube/Meta/Meta.hpp>

using namespace kF;
using namespace kF::Literal;

static void ConstructIntReference(benchmark::State &state)
{
    for (auto _ : state) {
        benchmark::DoNotOptimize(std::int64_t { state.range() });
    }
}
BENCHMARK(ConstructIntReference)->Arg(42);


static void ConstructInt(benchmark::State &state)
{
    for (auto _ : state) {
        benchmark::DoNotOptimize(Var { state.range() });
    }
}
BENCHMARK(ConstructInt)->Arg(42);
