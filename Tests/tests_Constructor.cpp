/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Unit tests of Constructor
 */

#include <memory>

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

TEST(Constructor, MoveUniqueInt)
{
    struct Foo
    {
        Foo(std::unique_ptr<int> &&x_) : x(std::move(x_)) {}

        std::unique_ptr<int> x;
    };

    Meta::Resolver::Clear();
    Meta::Factory<Foo>::Register("foo"_hash);
    Meta::Factory<Foo>::RegisterConstructor<std::unique_ptr<int> &&>();
    auto type = Meta::Factory<Foo>::Resolve();

    auto ctor = type.findConstructor<std::unique_ptr<int>>();
    ASSERT_TRUE(ctor);
    ASSERT_EQ(ctor, type.findConstructor(std::vector<Meta::Type> { Meta::Factory<std::unique_ptr<int>>::Resolve() }));

    auto tmp = std::make_unique<int>(42);
    Var instance = ctor.invoke(std::move(tmp));
    ASSERT_TRUE(instance);
    ASSERT_FALSE(tmp);
    ASSERT_EQ(instance.type(), type);
    ASSERT_EQ(*instance.as<Foo>().x, 42);

    tmp = std::make_unique<int>(84);
    const auto &constTmp = tmp;
    instance = ctor.invoke(constTmp); // Try to break the move semantics
    ASSERT_TRUE(instance);
    ASSERT_FALSE(constTmp);
    ASSERT_EQ(instance.type(), type);
    ASSERT_EQ(*instance.as<Foo>().x, 84);
}

TEST(Constructor, AdvancedConstructor)
{
    struct Foo
    {
        Foo(int x_, const std::string &y_, const float &z_) : x(x_), y(y_), z(z_) {}

        int x;
        std::string y;
        float z;
    };

    Meta::Resolver::Clear();
    Meta::RegisterMetadata();
    Meta::Factory<Foo>::Register("foo"_hash);
    Meta::Factory<Foo>::RegisterConstructor<int, const std::string &, const float &>();
    auto type = Meta::Factory<Foo>::Resolve();

    auto ctor = type.findConstructor<int, const std::string &, const float &>();
    ASSERT_TRUE(ctor);
    ASSERT_EQ(ctor, type.findConstructor(std::vector<Meta::Type> {
        Meta::Factory<int>::Resolve(),
        Meta::Factory<std::string>::Resolve(),
        Meta::Factory<float>::Resolve()
    }));

    Var instance = ctor.invoke(42.0f, std::string("azerty"), 32ul);
    ASSERT_TRUE(instance);
    ASSERT_EQ(instance.type(), type);
    ASSERT_EQ(instance.as<Foo>().x, 42);
    ASSERT_EQ(instance.as<Foo>().y, "azerty");
    ASSERT_EQ(instance.as<Foo>().z, 32.0f);
}