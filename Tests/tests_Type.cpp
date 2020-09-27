/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Unit tests of Type
 */

#include <memory>

#include <gtest/gtest.h>

#include <Kube/Meta/Meta.hpp>

using namespace kF;

TEST(Type, BasicsSemantics)
{
    Meta::Type t1;
    Meta::Type t2 = Meta::Factory<int>::Resolve();
    Meta::Type t3(t2);

    ASSERT_FALSE(t1.operator bool());
    ASSERT_TRUE(t2.operator bool());
    ASSERT_TRUE(t3.operator bool());
    t1 = t2;
    ASSERT_TRUE(t1.operator bool());
}

TEST(Type, BasicComparisons)
{
    auto t1 = Meta::Factory<int>::Resolve();
    auto t2 = Meta::Factory<const int &>::Resolve();
    auto t3 = Meta::Factory<float>::Resolve();

    ASSERT_TRUE(t1 == t2);
    ASSERT_TRUE(t1 != t3);
    ASSERT_TRUE(t2 != t3);
}

TEST(Type, MetaSemantics)
{
    using FooPtr = std::shared_ptr<double>;

    Meta::Resolver::Clear();
    auto fact = Meta::Factory<FooPtr>::Resolve();

    ASSERT_EQ(fact.typeID(), typeid(FooPtr));
    ASSERT_EQ(fact.name(), 0);
    ASSERT_EQ(fact.typeSize(), sizeof(FooPtr));
    ASSERT_EQ(fact.isSmallOptimized(), Meta::Internal::IsVarSmallOptimized<FooPtr>);
    ASSERT_FALSE(fact.isVoid());
    ASSERT_FALSE(fact.isIntegral());
    ASSERT_FALSE(fact.isFloating());
    ASSERT_FALSE(fact.isDouble());

    ASSERT_TRUE(fact.isDefaultConstructible());
    auto instance = fact.defaultConstruct();
    ASSERT_TRUE(instance);
    ASSERT_EQ(instance.type(), fact);
    ASSERT_EQ(instance.as<FooPtr>().get(), nullptr);

    auto &foo = instance.as<FooPtr>() = std::make_shared<double>(84.0);
    ASSERT_EQ(foo.use_count(), 1);

    ASSERT_TRUE(fact.isCopyConstructible());
    auto copy = fact.copyConstruct(instance.data());
    ASSERT_TRUE(copy);
    ASSERT_EQ(copy.type(), fact);
    ASSERT_EQ(*copy.as<FooPtr>(), 84.0);
    ASSERT_EQ(foo.use_count(), 2);

    ASSERT_TRUE(fact.isMoveConstructible());
    auto move = fact.moveConstruct(copy.data());
    ASSERT_TRUE(move);
    ASSERT_EQ(move.type(), fact);
    ASSERT_EQ(*move.as<FooPtr>(), 84.0);
    ASSERT_EQ(foo.use_count(), 2);
    ASSERT_FALSE(copy.as<FooPtr>());

    ASSERT_TRUE(fact.isCopyAssignable());
    fact.copyAssign(copy.data(), move.data());
    ASSERT_EQ(copy.type(), fact);
    ASSERT_EQ(*copy.as<FooPtr>(), 84.0);
    ASSERT_EQ(foo.use_count(), 3);
    ASSERT_TRUE(move.as<FooPtr>());

    ASSERT_TRUE(fact.isMoveAssignable());
    fact.moveAssign(move.data(), copy.data());
    ASSERT_EQ(move.type(), fact);
    ASSERT_EQ(*move.as<FooPtr>(), 84.0);
    ASSERT_EQ(foo.use_count(), 2);
    ASSERT_FALSE(copy.as<FooPtr>());

    ASSERT_TRUE(fact.isBoolConvertible());
    ASSERT_TRUE(fact.toBool(instance.data()));
    ASSERT_FALSE(fact.toBool(copy.data()));
    ASSERT_TRUE(fact.toBool(move.data()));

    move.destruct();
    ASSERT_EQ(foo.use_count(), 1);

    ASSERT_FALSE(fact.hasOperator<Meta::UnaryOperator::Minus>());
    ASSERT_FALSE(fact.hasOperator<Meta::BinaryOperator::Addition>());
    ASSERT_FALSE(fact.hasOperator<Meta::BinaryOperator::Substraction>());
    ASSERT_FALSE(fact.hasOperator<Meta::BinaryOperator::Multiplication>());
    ASSERT_FALSE(fact.hasOperator<Meta::BinaryOperator::Division>());
    ASSERT_FALSE(fact.hasOperator<Meta::BinaryOperator::Modulo>());
    ASSERT_FALSE(fact.hasOperator<Meta::AssignmentOperator::Addition>());
    ASSERT_FALSE(fact.hasOperator<Meta::AssignmentOperator::Substraction>());
    ASSERT_FALSE(fact.hasOperator<Meta::AssignmentOperator::Multiplication>());
    ASSERT_FALSE(fact.hasOperator<Meta::AssignmentOperator::Division>());
    ASSERT_FALSE(fact.hasOperator<Meta::AssignmentOperator::Modulo>());
}

// TEST(Type, Operator)
// {

// }