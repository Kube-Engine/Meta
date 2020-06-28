/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Unit tests of Converter
 */

#include <gtest/gtest.h>

#include <Kube/Meta/Meta.hpp>

using namespace kF;

#define CONVERTER_TEST(TestName, From, fromValue, To, toValue, converterFunc) \
TEST(Converter, TestName) \
{ \
    Meta::Resolver::Clear(); \
    Meta::Factory<From>::Register(Hash(#TestName)); \
    Meta::Factory<From>::RegisterConverter<To, converterFunc>(); \
    Var x { From(fromValue) }; \
    auto conv = Meta::Factory<From>::Resolve().findConverter(Meta::Factory<To>::Resolve()); \
    ASSERT_TRUE(conv); \
    ASSERT_EQ(conv.convertType(), Meta::Factory<To>::Resolve()); \
    auto res = conv.invoke(x); \
    ASSERT_TRUE(res); \
    ASSERT_EQ(res.type(), Meta::Factory<To>::Resolve()); \
    ASSERT_EQ(res.as<To>(), toValue); \
}

CONVERTER_TEST(IntToDoubleAuto,
    int, 42,
    double, 42.0,
    nullptr
)

CONVERTER_TEST(IntToStringFunction,
    int, 42,
    std::string, "42",
    static_cast<std::string(*)(int)>(&std::to_string)
)

CONVERTER_TEST(IntToStringLambda,
    int, 42,
    std::string, "42",
    [](auto x) { return std::to_string(x); }
)
