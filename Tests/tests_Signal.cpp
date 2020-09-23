/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Unit tests of Signal
 */

#include <atomic>

#include <gtest/gtest.h>

#include <Kube/Meta/Meta.hpp>

using namespace kF;
using namespace kF::Literal;

#include <iostream>

TEST(Signal, NoParameters)
{
    struct Foo { void signal(void) { } };

    Meta::Resolver::Clear();
    Meta::Factory<Foo>::Register("foo"_hash);
    Meta::Factory<Foo>::RegisterSignal<&Foo::signal>("signal"_hash);

    int x = 0;
    Foo foo;
    auto type = Meta::Factory<Foo>::Resolve();
    auto sig = type.findSignal<&Foo::signal>();
    ASSERT_EQ(sig, type.findSignal("signal"_hash));
    ASSERT_EQ(sig.argsCount(), 0);

    auto conn = sig.connect(&foo, [&x] {
        ++x;
    });
    sig.emit(&foo);
    ASSERT_EQ(x, 1);
    sig.emit(&foo);
    ASSERT_EQ(x, 2);
    conn.disconnect();
    sig.emit(&foo);
    ASSERT_EQ(x, 2);

#if KUBE_DEBUG_BUILD
    // Too many arguments
    ASSERT_ANY_THROW(sig.emit(&foo, 42));
    // Mismatching connect
    ASSERT_ANY_THROW(sig.connect(&foo, [](int) {}).disconnect());
#endif
}

TEST(Signal, MultipleParameters)
{
    struct Foo { void signal(int, float) { } };

    Meta::Resolver::Clear();
    Meta::RegisterMetadata();
    Meta::Factory<Foo>::Register("foo"_hash);
    Meta::Factory<Foo>::RegisterSignal<&Foo::signal>("signal"_hash);

    int x = 0;
    float y = 0;
    Foo foo;
    auto type = Meta::Factory<Foo>::Resolve();
    auto sig = type.findSignal<&Foo::signal>();
    ASSERT_EQ(sig, type.findSignal("signal"_hash));
    ASSERT_EQ(sig.argsCount(), 2);

    // Perfect slot signature
    auto conn = sig.connect(&foo, [&x, &y](int x_, float y_) {
        x = x_;
        y = y_;
    });
    sig.emit(&foo, 42, 42.5f);
    ASSERT_EQ(x, 42);
    ASSERT_EQ(y, 42.5f);
    sig.emit(&foo, 10.25f, 6ul);
    ASSERT_EQ(x, 10);
    ASSERT_EQ(y, 6.0f);
    conn.disconnect();
    sig.emit(&foo, 0, 0.0f);
    ASSERT_EQ(x, 10);
    ASSERT_EQ(y, 6.0f);

    // Convertible slot signature
    conn = sig.connect(&foo, [&x, &y](float x_, int y_) {
        x = static_cast<int>(x_);
        y = static_cast<float>(y_);
    });
    sig.emit(&foo, 42, 24.5f);
    ASSERT_EQ(x, 42);
    ASSERT_EQ(y, 24.0f);
    conn.disconnect();
    sig.emit(&foo, 0, 0);
    ASSERT_EQ(x, 42);
    ASSERT_EQ(y, 24.0f);

    // Non-convertible slot signature
    conn = sig.connect(&foo, [](const std::string &, Foo) {});

#if KUBE_DEBUG_BUILD
    // Invalid arguments
    ASSERT_ANY_THROW(sig.emit(&foo, 0, 0));
    // Not enough arguments
    ASSERT_ANY_THROW(sig.emit(&foo, 42));
    // Too many arguments
    ASSERT_ANY_THROW(sig.emit(&foo, 42, 42.5f, 43));
    // Mismatching connect
    ASSERT_ANY_THROW(sig.connect(&foo, [](int, float, int) {}).disconnect());
#endif
}
