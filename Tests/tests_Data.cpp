/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Unit tests of Data
 */

#include <gtest/gtest.h>

#include <Kube/Meta/Meta.hpp>

using namespace kF;

template<typename Type>
struct GetSetCopy
{
    GetSetCopy(Type &&x_) : x(x_) {}

    operator const Type&() const { return x; }

    const Type &get(void) { return x; }
    void set(const Type &x_) { x = x_; }

    Type x;
};

template<typename Type>
struct GetSetMove
{
    GetSetMove(Type &&x_) : x(x_) {}

    operator const Type&() const { return x; }

    const Type &get(void) { return x; }
    void set(Type &&x_) { x = std::move(x_); }

    Type x;
};

template<typename Type>
struct GetSetStaticCopy
{
    static Type &Get(void) noexcept {
        static Type x {};
        return x;
    }

    static void Set(const Type &x) {
        Get() = x;
    }
};

template<typename Type>
struct GetSetStaticMove
{
    static Type &Get(void) noexcept {
        static Type x {};
        return x;
    }

    static void Set(Type &&x) {
        Get() = std::move(x);
    }
};

#define CONVERTER_TEST_GETSET(TestName, GetSetImpl, Type, startValue, setValue) \
TEST(Data, TestName) \
{ \
    using GetSetType = GetSetImpl<Type>; \
    Meta::Resolver::Clear(); \
    Meta::Factory<GetSetType>::Register(Hash(#TestName)); \
    Meta::Factory<GetSetType>::RegisterData<&GetSetType::get, &GetSetType::set>(Hash("data")); \
    Var x { startValue }; \
    auto data = Meta::Factory<GetSetType>::Resolve().findData(Hash("data")); \
    ASSERT_TRUE(data); \
    ASSERT_EQ(data.isStatic(), false); \
    ASSERT_EQ(data.name(), Hash("data")); \
    auto res = data.get(x); \
    ASSERT_TRUE(res); \
    ASSERT_EQ(res.type(), Meta::Factory<Type>::Resolve()); \
    ASSERT_EQ(res.as<Type>(), startValue); \
    res = data.set(x, setValue); \
    ASSERT_TRUE(res); \
    ASSERT_TRUE(res.isVoid()); \
    res = data.get(x); \
    ASSERT_TRUE(res); \
    ASSERT_EQ(res.type().typeID(), Meta::Factory<Type>::Resolve().typeID()); \
    ASSERT_EQ(res.as<Type>(), setValue); \
}

#define CONVERTER_TEST_GETSET_STATIC(TestName, GetSetImpl, Type, startValue, setValue) \
TEST(Data, TestName) \
{ \
    using GetSetType = GetSetImpl<Type>; \
    Meta::Resolver::Clear(); \
    Meta::Factory<GetSetType>::Register(Hash(#TestName)); \
    Meta::Factory<GetSetType>::RegisterData<&GetSetType::Get, &GetSetType::Set>(Hash("data")); \
    GetSetType::Set(startValue); \
    auto data = Meta::Factory<GetSetType>::Resolve().findData(Hash("data")); \
    ASSERT_TRUE(data); \
    ASSERT_EQ(data.isStatic(), true); \
    ASSERT_EQ(data.name(), Hash("data")); \
    auto res = data.get(); \
    ASSERT_TRUE(res); \
    ASSERT_EQ(res.type(), Meta::Factory<Type>::Resolve()); \
    ASSERT_EQ(res.as<Type>(), startValue); \
    res = data.set(setValue); \
    ASSERT_TRUE(res); \
    ASSERT_TRUE(res.isVoid()); \
    res = data.get(); \
    ASSERT_TRUE(res); \
    ASSERT_EQ(res.type().typeID(), Meta::Factory<Type>::Resolve().typeID()); \
    ASSERT_EQ(res.as<Type>(), setValue); \
}

struct MoveOnlyFoo
{
    MoveOnlyFoo(void) = default;
    MoveOnlyFoo(int value) : x(value) {}
    MoveOnlyFoo(MoveOnlyFoo &&other) = default;
    MoveOnlyFoo &operator=(MoveOnlyFoo &&other) = default;

    operator const int &(void) const noexcept { return x; }

    int x = 0;
};

CONVERTER_TEST_GETSET(GetSetCopyInt, GetSetCopy, int, 42, 84)
CONVERTER_TEST_GETSET(GetSetMoveInt, GetSetMove, int, 42, 84)
CONVERTER_TEST_GETSET_STATIC(GetSetStaticCopyInt, GetSetStaticCopy, int, 42, 84)
CONVERTER_TEST_GETSET_STATIC(GetSetStaticMoveInt, GetSetStaticMove, int, 42, 84)

CONVERTER_TEST_GETSET(GetSetCopyString, GetSetCopy, std::string, std::string("hello"), std::string("world"))
CONVERTER_TEST_GETSET(GetSetMoveString, GetSetMove, std::string, std::string("hello"), std::string("world"))
CONVERTER_TEST_GETSET_STATIC(GetSetStaticCopyString, GetSetStaticCopy, std::string, std::string("hello"), std::string("world"))
CONVERTER_TEST_GETSET_STATIC(GetSetStaticMoveString, GetSetStaticMove, std::string, std::string("hello"), std::string("world"))

CONVERTER_TEST_GETSET(GetSetCopyVector, GetSetCopy, std::vector<int>, (std::vector<int> { 0, 1, 2, 3 }), (std::vector<int> { 3, 2, 1, 0 }))
CONVERTER_TEST_GETSET(GetSetMoveVector, GetSetMove, std::vector<int>, (std::vector<int> { 0, 1, 2, 3 }), (std::vector<int> { 3, 2, 1, 0 }))
CONVERTER_TEST_GETSET_STATIC(GetSetStaticCopyVector, GetSetStaticCopy, std::vector<int>, (std::vector<int> { 0, 1, 2, 3 }), (std::vector<int> { 3, 2, 1, 0 }))
CONVERTER_TEST_GETSET_STATIC(GetSetStaticMoveVector, GetSetStaticMove, std::vector<int>, (std::vector<int> { 0, 1, 2, 3 }), (std::vector<int> { 3, 2, 1, 0 }))

CONVERTER_TEST_GETSET(GetSetMoveMoveOnlyFoo, GetSetMove, MoveOnlyFoo, MoveOnlyFoo(42), MoveOnlyFoo(84))
CONVERTER_TEST_GETSET_STATIC(GetSetStaticMoveMoveOnlyFoo, GetSetStaticMove, MoveOnlyFoo, MoveOnlyFoo(42), MoveOnlyFoo(84))
