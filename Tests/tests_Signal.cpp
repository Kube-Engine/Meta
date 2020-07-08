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

    // Too many arguments
    ASSERT_ANY_THROW(sig.emit(&foo, 42));

    // Mismatching connect
    ASSERT_ANY_THROW(sig.connect(&foo, [](int) {}));
}

TEST(Signal, MultipleParameters)
{
    struct Foo { void signal(int, float) { } };

    Meta::Resolver::Clear();
    Meta::RegisterMetadata();
    Meta::Factory<Foo>::Register("foo"_hash);
    Meta::Factory<Foo>::RegisterSignal<&Foo::signal>("signal"_hash, { "x"_hash, "y"_hash });

    int x = 0;
    float y = 0;
    Foo foo;
    auto type = Meta::Factory<Foo>::Resolve();
    auto sig = type.findSignal<&Foo::signal>();
    ASSERT_EQ(sig, type.findSignal("signal"_hash));
    ASSERT_EQ(sig.argsCount(), 2);
    ASSERT_EQ(sig.arguments()[0], "x"_hash);
    ASSERT_EQ(sig.arguments()[1], "y"_hash);

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
    ASSERT_ANY_THROW(sig.emit(&foo, 0, 0));

    // Not enough many arguments
    ASSERT_ANY_THROW(sig.emit(&foo, 42));

    // Too many arguments
    ASSERT_ANY_THROW(sig.emit(&foo, 42, 42.5f, 43));

    // Mismatching connect
    ASSERT_ANY_THROW(sig.connect(&foo, [](int, float, int) {}));
}

// TEST(Signal, DelayedSlot)
// {
//     struct Foo { void signal(const std::shared_ptr<int> &) { } };

//     Meta::Resolver::Clear();
//     Meta::RegisterMetadata();
//     Meta::Factory<Foo>::Register("foo"_hash);
//     Meta::Factory<Foo>::RegisterSignal<&Foo::signal>("signal"_hash, { "ptr"_hash });

//     auto counter = std::make_shared<int>(42);
//     Foo foo;
//     auto type = Meta::Factory<Foo>::Resolve();
//     auto sig = type.findSignal<&Foo::signal>();
//     ASSERT_EQ(sig, type.findSignal("signal"_hash));
//     ASSERT_EQ(sig.argsCount(), 1);
//     ASSERT_EQ(sig.arguments()[0], "ptr"_hash);

//     std::atomic<bool> preWork = false;
//     bool postWork = false;
//     std::thread thd([&sig, &foo, &preWork, &postWork] {
//         bool running = true;
//         auto conn = sig.connect(&foo, [&running](const std::shared_ptr<int> &ptr) {
//             running = false;
//             ++*ptr;
//         });
//         preWork = true;
//         while (preWork);
//         while (running)
//             Meta::Signal::ProcessDelayedSlots();
//         postWork = true;
//     });
//     while (!preWork);
//     sig.emit(&foo, counter);
//     ASSERT_EQ(counter.use_count(), 2);
//     ASSERT_EQ(*counter, 42);
//     preWork = false;
//     if (thd.joinable())
//         thd.join();
//     ASSERT_EQ(*counter, 43);
//     ASSERT_TRUE(postWork);
// }