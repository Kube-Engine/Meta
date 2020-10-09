/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Unit tests of Signal
 */

#include <atomic>

#include <gtest/gtest.h>

#include <Kube/Meta/Meta.hpp>

using namespace kF;
using namespace kF::Literal;

TEST(Signal, NoParameters)
{
    struct Foo { void signal(void) { } };

    Meta::Resolver::Clear();
    Meta::Factory<Foo>::Register("foo"_hash);
    Meta::Factory<Foo>::RegisterSignal<&Foo::signal>("signal"_hash);

    auto type = Meta::Factory<Foo>::Resolve();
    auto sig = type.findSignal<&Foo::signal>();
    ASSERT_EQ(sig, type.findSignal("signal"_hash));
    ASSERT_EQ(sig.argsCount(), 0);
}

TEST(Signal, MultipleParameters)
{
    struct Foo { void signal(int, float) { } };

    Meta::Resolver::Clear();
    Meta::RegisterMetadata();
    Meta::Factory<Foo>::Register("foo"_hash);
    Meta::Factory<Foo>::RegisterSignal<&Foo::signal>("signal"_hash);

    auto type = Meta::Factory<Foo>::Resolve();
    auto sig = type.findSignal<&Foo::signal>();
    ASSERT_EQ(sig, type.findSignal("signal"_hash));
    ASSERT_EQ(sig.argsCount(), 2);
}
