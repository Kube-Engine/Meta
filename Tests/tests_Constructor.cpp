/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Unit tests of Constructor
 */

#include <gtest/gtest.h>

#include <Kube/Meta/Meta.hpp>

using namespace kF;
using namespace kF::Literal;

TEST(Constructor, CopyInt)
{
    struct Foo
    {
        Foo(int x_) : x(x_) {}

        int x;
    };

    Meta::Resolver::Clear();
    Meta::Factory<Foo>::Register("foo"_hash);
    Meta::Factory<Foo>::RegisterConstructor<int>();
    auto type = Meta::Factory<Foo>::Resolve();

    auto ctor = type.findConstructor<int>();
    ASSERT_TRUE(ctor);
    ASSERT_EQ(ctor, type.findConstructor(std::vector<Meta::Type> { Meta::Factory<int>::Resolve() }));

    Var instance = ctor.invoke(42);
    ASSERT_TRUE(instance);
    ASSERT_EQ(instance.type(), type);
    ASSERT_EQ(instance.as<Foo>().x, 42);
}
