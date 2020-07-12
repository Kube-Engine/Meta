/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Unit tests of Var
 */

#include <memory>

#include <gtest/gtest.h>

#include <Kube/Meta/Meta.hpp>

using namespace kF;
using namespace kF::Literal;

TEST(Var, NullVar)
{
    Var var;

    ASSERT_FALSE(var.operator bool());
}

TEST(Var, TrivialEmplace)
{
    Var var;

    ASSERT_FALSE(var.operator bool());
    var.emplace<int>(42);
    ASSERT_TRUE(var.operator bool());
    ASSERT_EQ(var.as<int>(), 42);
}

TEST(Var, NonTrivialEmplace)
{
    Var var;

    ASSERT_FALSE(var.operator bool());
    var.emplace<std::string>("abc");
    ASSERT_TRUE(var.operator bool());
    ASSERT_EQ(var.as<std::string>(), "abc");
}

TEST(Var, ValueAssignBasics)
{
    using Array = std::array<int, Meta::Internal::TrivialTypeSizeLimit * 2>;

    Var var;

    var.assign(42);
    ASSERT_EQ(var.storageType(), Var::StorageType::ValueTrivial);
    ASSERT_EQ(var.as<int>(), 42);

    var.assign(std::string("abc"));
    ASSERT_EQ(var.storageType(), Var::StorageType::ValueTrivial);
    ASSERT_EQ(var.as<std::string>(), "abc");

    var.assign(std::vector<int> { 42 });
    ASSERT_EQ(var.storageType(), Var::StorageType::ValueTrivial);
    ASSERT_EQ(var.as<std::vector<int>>()[0], 42);

    var.assign(Array { 42 });
    ASSERT_EQ(var.storageType(), Var::StorageType::Value);
    ASSERT_EQ(var.as<Array>()[0], 42);
}

TEST(Var, RefAssignBasics)
{
    Var var;

    int a = 42;
    var.assign(a);
    ASSERT_EQ(var.storageType(), Var::StorageType::ReferenceVolatile);
    ASSERT_EQ(++var.as<int>(), 43);
    ASSERT_EQ(a, 43);

    std::string b = "abc";
    var.assign(b);
    ASSERT_EQ(var.storageType(), Var::StorageType::ReferenceVolatile);
    ASSERT_EQ(var.as<std::string>().append("de"), "abcde");
    ASSERT_EQ(b, "abcde");

    const auto &aRef = a;
    var.assign(aRef);
    ASSERT_EQ(var.storageType(), Var::StorageType::ReferenceConstant);
    ASSERT_EQ(++var.as<int>(), 44);
    ASSERT_EQ(a, 44);

    const auto &bRef = b;
    var.assign(bRef);
    ASSERT_EQ(var.storageType(), Var::StorageType::ReferenceConstant);
    ASSERT_EQ(var.as<std::string>().append("fgh"), "abcdefgh");
    ASSERT_EQ(b, "abcdefgh");
}

TEST(Var, TrickyAssign)
{
    auto x = Var::Emplace<int>(42);
    auto tmpRef = Var::Assign(x);
    auto ref = Var::Assign(std::move(tmpRef));

    ++ref.as<int>();
    ASSERT_FALSE(tmpRef);
    ASSERT_EQ(x.as<int>(), 43);
}

TEST(Var, TrickyDeepCopy)
{
    auto x = Var::Emplace<int>(42);
    auto tmpRef = Var::Assign(x);
    Var value = tmpRef;

    ASSERT_EQ(++value.as<int>(), 43);
    ASSERT_TRUE(tmpRef);
    ASSERT_EQ(x.as<int>(), 42);
    value = x;
    ASSERT_EQ(++value.as<int>(), 43);
    ASSERT_TRUE(tmpRef);
    ASSERT_EQ(x.as<int>(), 42);
}

TEST(Var, DestructorCheck)
{
    auto ptr = std::make_shared<int>(42);

    ASSERT_TRUE(ptr.unique());
    {
        Var var;
        ASSERT_FALSE(var.operator bool());
        var.emplace<std::shared_ptr<int>>(ptr);
        ASSERT_TRUE(var.operator bool());
        ASSERT_FALSE(ptr.unique());
    }
    ASSERT_TRUE(ptr.unique());
}

TEST(Var, ConversionBasics)
{
    Meta::Resolver::Clear();
    Meta::Factory<int>::Register("int"_hash);
    Meta::Factory<int>::RegisterConverter<float>();
    Meta::Factory<int>::RegisterConverter<std::string, static_cast<std::string(*)(int)>(std::to_string)>();
    Var var, tmp;

    var.emplace<int>(42);
    ASSERT_EQ(var.as<int>(), 42);
    tmp = var.convert<float>();
    ASSERT_EQ(tmp.as<float>(), 42.0f);
    tmp = var.convert<std::string>();
    ASSERT_EQ(tmp.as<std::string>(), "42");
}

TEST(Var, BinaryOperators)
{
    Meta::Resolver::Clear();
    Meta::RegisterMetadata();

    // Int only
    auto res = Var::Emplace<int>(40) + Var::Emplace<int>(2);
    ASSERT_EQ(res.cast<int>(), 42);
    res = res - Var::Emplace<int>(40);
    ASSERT_EQ(res.cast<int>(), 2);
    res = res * Var::Emplace<int>(3);
    ASSERT_EQ(res.cast<int>(), 6);
    res = res / Var::Emplace<int>(2);
    ASSERT_EQ(res.cast<int>(), 3);
    res = res % Var::Emplace<int>(2);
    ASSERT_EQ(res.cast<int>(), 1);

    // Float only
    res = Var::Emplace<float>(40.5f) + Var::Emplace<float>(2);
    ASSERT_EQ(res.cast<float>(), 42.5f);
    res = res - Var::Emplace<float>(40.0f);
    ASSERT_EQ(res.cast<float>(), 2.5f);
    res = res * Var::Emplace<float>(2.0f);
    ASSERT_EQ(res.cast<float>(), 5.0f);
    res = res / Var::Emplace<float>(2.0f);
    ASSERT_EQ(res.cast<float>(), 2.5f);
    res = res % Var::Emplace<float>(30.0f);
    ASSERT_EQ(res.cast<float>(), 2.0f);

    // Mixed types
    res = Var::Emplace<float>(42.5f) + Var::Emplace<char>(8);
    ASSERT_EQ(res.cast<float>(), 50.5f);
    res = Var::Emplace<std::int64_t>(123) - Var::Emplace<double>(1.5f);
    ASSERT_EQ(res.cast<double>(), 121.5);
    res = Var::Emplace<std::int8_t>(42) * Var::Emplace<float>(0.5f);
    ASSERT_EQ(res.cast<float>(), 21);
    res = Var::Emplace<std::int8_t>(42) / Var::Emplace<std::int16_t>(2);
    ASSERT_EQ(res.cast<std::int8_t>(), 21);
    res = Var::Emplace<double>(3) % Var::Emplace<float>(2);
    ASSERT_EQ(res.cast<double>(), 1.0);
}

TEST(Var, AssignmentOperators)
{
    Meta::Resolver::Clear();
    Meta::RegisterMetadata();

    // Int only
    auto res = Var::Emplace<int>(40) + Var::Emplace<int>(2);
    ASSERT_EQ(res.cast<int>(), 42);
    res -= Var::Emplace<int>(40);
    ASSERT_EQ(res.cast<int>(), 2);
    res *= Var::Emplace<int>(3);
    ASSERT_EQ(res.cast<int>(), 6);
    res /= Var::Emplace<int>(2);
    ASSERT_EQ(res.cast<int>(), 3);
    res %= Var::Emplace<int>(2);
    ASSERT_EQ(res.cast<int>(), 1);

    // Float only
    res = Var::Emplace<float>(40.5f);
    res += Var::Emplace<float>(2);
    ASSERT_EQ(res.cast<float>(), 42.5f);
    res -= Var::Emplace<float>(40.0f);
    ASSERT_EQ(res.cast<float>(), 2.5f);
    res *= Var::Emplace<float>(2.0f);
    ASSERT_EQ(res.cast<float>(), 5.0f);
    res /= Var::Emplace<float>(2.0f);
    ASSERT_EQ(res.cast<float>(), 2.5f);
    res %= Var::Emplace<float>(30.0f);
    ASSERT_EQ(res.cast<float>(), 2.0f);

    // Mixed types
    res = Var::Emplace<float>(42.5f);
    res += Var::Emplace<char>(8);
    ASSERT_EQ(res.cast<float>(), 50.5f);
    res = Var::Emplace<std::int64_t>(123);
    res -= Var::Emplace<double>(1.5f);
    ASSERT_EQ(res.cast<std::int64_t>(), 121);
    res = Var::Emplace<std::int8_t>(42);
    res *= Var::Emplace<float>(0.5f);
    ASSERT_EQ(res.cast<std::int8_t>(), 21);
    res = Var::Emplace<std::int8_t>(42);
    res /= Var::Emplace<std::int16_t>(2);
    ASSERT_EQ(res.cast<std::int8_t>(), 21);
    res = Var::Emplace<double>(3);
    res %= Var::Emplace<float>(2);
    ASSERT_EQ(res.cast<double>(), 1.0);
}

TEST(Var, PointerHandling)
{
    int array[4] { 42030, 12345412, 23321 };
    Var var(static_cast<int *>(array));

    ASSERT_EQ(*var.as<int *>(), 42030);
    ASSERT_EQ(*(var + 1).as<int *>(), 12345412);
    var += 2;
    ASSERT_EQ(*var.as<int *>(), 23321);
    ASSERT_EQ(*(var - 1).as<int *>(), 12345412);
    var -= 1;
    ASSERT_EQ(*var.as<int *>(), 12345412);
}

TEST(Var, ArrayHandling)
{
    int array[4] { 42030, 12345412, 23321 };
    Var var(array);

    ASSERT_EQ(*var.as<int *>(), 42030);
    ASSERT_EQ(*(var + 1).as<int *>(), 12345412);
    var += 2;
    ASSERT_EQ(*var.as<int *>(), 23321);
    ASSERT_EQ(*(var - 1).as<int *>(), 12345412);
    var -= 1;
    ASSERT_EQ(*var.as<int *>(), 12345412);
}