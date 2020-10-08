/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Meta Signal benchmark
 */

#include <chrono>

#include <benchmark/benchmark.h>

#include <Kube/Meta/Meta.hpp>
#include <Kube/Core/FlatString.hpp>

using namespace kF;
using namespace kF::Literal;

constexpr auto ShortStringValue = "hello";
constexpr auto LongStringValue = "0123456789ABCDEFGHIJ";

// Benchmark generator builtins
#define GENERATE_BENCHMARK(TestName, body) \
static void TestName(benchmark::State &state) \
    body \
BENCHMARK(TestName);//->UseManualTime();

#define GENERATE_EXPR_BENCHMARK(TestName, expr) \
GENERATE_BENCHMARK(TestName, \
{ \
    for (auto _ : state) { \
        expr \
    } \
})

#define GENERATE_INIT_EXPR_BENCHMARK(TestName, init, expr) \
GENERATE_BENCHMARK(TestName, \
{ \
    init \
    for (auto _ : state) { \
        expr \
    } \
})

#define GENERATE_BENCHMARK_AND_REFERENCE(GeneratorName, Name, Type, value) \
    GENERATE_##GeneratorName##_REFERENCE_BENCHMARK(Name, Type, value) \
    GENERATE_##GeneratorName##_BENCHMARK(Name, Type, value)

#define GENERATE_REFERENCED_BENCHMARKS(GeneratorName) \
    GENERATE_BENCHMARK_AND_REFERENCE(GeneratorName, Int64, std::int64_t, 42) \
    GENERATE_BENCHMARK_AND_REFERENCE(GeneratorName, StdStringShort, std::string, ShortStringValue) \
    GENERATE_BENCHMARK_AND_REFERENCE(GeneratorName, StdStringLong, std::string, LongStringValue) \
    GENERATE_BENCHMARK_AND_REFERENCE(GeneratorName, FlatStringShort, Core::FlatString, ShortStringValue) \
    GENERATE_BENCHMARK_AND_REFERENCE(GeneratorName, FlatStringLong, Core::FlatString, LongStringValue)

// Specific benchmark generators
#define GENERATE_CONSTRUCT_BENCHMARK(Name, Type, value) \
    GENERATE_EXPR_BENCHMARK(Construct##Name, \
        benchmark::DoNotOptimize(Var::Emplace<Type>(value)); \
    )

#define GENERATE_CONSTRUCT_REFERENCE_BENCHMARK(Name, Type, value) \
    GENERATE_EXPR_BENCHMARK(Construct##Name##Reference, \
        benchmark::DoNotOptimize(Type { value }); \
    )

GENERATE_REFERENCED_BENCHMARKS(CONSTRUCT)

#define GENERATE_COPY_CONSTRUCT_BENCHMARK(Name, Type, value) \
    GENERATE_INIT_EXPR_BENCHMARK(Copy##Construct##Name, \
        auto toCopy { Var::Emplace<Type>(value) };, \
        benchmark::DoNotOptimize(Var(toCopy)); \
    )

#define GENERATE_COPY_CONSTRUCT_REFERENCE_BENCHMARK(Name, Type, value) \
    GENERATE_INIT_EXPR_BENCHMARK(Copy##Construct##Name##Reference, \
        Type toCopy { value };, \
        benchmark::DoNotOptimize(Type(toCopy)); \
    )

GENERATE_REFERENCED_BENCHMARKS(COPY_CONSTRUCT)

#define GENERATE_MOVE_CONSTRUCT_BENCHMARK(Name, Type, value) \
    GENERATE_EXPR_BENCHMARK(Move##Construct##Name, \
        auto x = Var::Emplace<Type>(value); \
        benchmark::DoNotOptimize(Var(std::move(x))); \
    )

#define GENERATE_MOVE_CONSTRUCT_REFERENCE_BENCHMARK(Name, Type, value) \
    GENERATE_EXPR_BENCHMARK(Move##Construct##Name##Reference, \
        Type x(value); \
        benchmark::DoNotOptimize(Type(std::move(x))); \
    )

GENERATE_REFERENCED_BENCHMARKS(MOVE_CONSTRUCT)