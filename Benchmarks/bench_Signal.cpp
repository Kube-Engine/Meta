/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Meta Signal benchmark
 */

#include <benchmark/benchmark.h>

#include <Kube/Meta/Meta.hpp>

using namespace kF;
using namespace kF::Literal;

static void ConnectDisconnect(benchmark::State &state)
{
    struct Foo { void signal(void) {} };

    static Meta::Signal sig;

    Foo foo;

    if (state.thread_index == 0) {
        Meta::Resolver::Clear();
        Meta::RegisterMetadata();
        Meta::Factory<Foo>::Register("foo"_hash);
        Meta::Factory<Foo>::RegisterSignal<&Foo::signal>("signal"_hash);
        sig = Meta::Factory<Foo>::Resolve().findSignal<&Foo::signal>();
    }

    for (auto _ : state) {
        benchmark::DoNotOptimize(sig.connect(&foo, [] {}));
    }
}
BENCHMARK(ConnectDisconnect);//->ThreadRange(1, 16);

static void DirectCallNoArguments_Reference(benchmark::State &state)
{
    int x = 0;
    std::function<int(void)> fct = [&x] {
        return ++x;
    };
    for (auto _ : state) {
        benchmark::DoNotOptimize(fct());
    }
}
BENCHMARK(DirectCallNoArguments_Reference);

static void DirectCallNoArguments(benchmark::State &state)
{
    struct Foo { void signal(void) {} };

    Meta::Resolver::Clear();
    Meta::RegisterMetadata();
    Meta::Factory<Foo>::Register("foo"_hash);
    Meta::Factory<Foo>::RegisterSignal<&Foo::signal>("signal"_hash);

    auto sig = Meta::Factory<Foo>::Resolve().findSignal<&Foo::signal>();
    Foo foo;

    int x = 0;
    auto conn = sig.connect(&foo, [&x] {
        ++x;
    });

    for (auto _ : state) {
        sig.emit(&foo);
    }
}
BENCHMARK(DirectCallNoArguments);

static void DirectCallIntFloatArguments_Reference(benchmark::State &state)
{
    int x = 0;
    float y = 0.0f;
    auto fct = [&x, &y](int x_, float y_) {
        x = x_;
        y = y_;
        return x;
    };

    for (auto _ : state) {
        benchmark::DoNotOptimize(fct(x + 1, y + 1.0f));
    }
}
BENCHMARK(DirectCallIntFloatArguments_Reference);

static void DirectCallIntFloatArguments(benchmark::State &state)
{
    struct Foo { void signal(int, float) {} };

    Meta::Resolver::Clear();
    Meta::RegisterMetadata();
    Meta::Factory<Foo>::Register("foo"_hash);
    Meta::Factory<Foo>::RegisterSignal<&Foo::signal>("signal"_hash);

    auto sig = Meta::Factory<Foo>::Resolve().findSignal<&Foo::signal>();
    Foo foo;

    int x = 0;
    float y = 0.0f;
    auto conn = sig.connect(&foo, [&x, &y](int x_, float y_) {
        x = x_;
        y = y_;
    });

    for (auto _ : state) {
        sig.emit(&foo, x + 1, y + 1.0f);
    }
}
BENCHMARK(DirectCallIntFloatArguments);