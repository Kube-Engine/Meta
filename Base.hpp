/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Meta internal helpers
 */

#pragma once

#include <type_traits>
#include <experimental/type_traits>
#include <typeindex>
#include <utility>
#include <tuple>
#include <functional>

#include <Kube/Core/Hash.hpp>
#include <Kube/Core/Assert.hpp>
#include <Kube/Core/Utils.hpp>

#include "Forward.hpp"

#ifndef KF_META_VAR_SMALL_OPTIMIZATION_SIZE
# define KF_META_VAR_SMALL_OPTIMIZATION_SIZE 16ul
#endif

namespace kF
{
    namespace Meta
    {
        /** @brief Unary meta operators */
        enum class UnaryOperator {
            Minus,
            Total
        };

        /** @brief Binary meta operators */
        enum class BinaryOperator {
            Addition,
            Substraction,
            Multiplication,
            Division,
            Modulo,
            Total
        };

        /** @brief Assignment meta operators */
        enum class AssignmentOperator {
            Addition,
            Substraction,
            Multiplication,
            Division,
            Modulo,
            Total
        };

        /** @brief Register all base metadata */
        void RegisterMetadata(void);

        /**
         * @brief Internal type resolvers and helpers
         */
        namespace Internal
        {
            template<typename Type, typename = void>
            struct FunctionDecomposer;

            /** @brief Describes a function */
            template<typename Return, typename ...Args>
            struct FunctionDecomposer<Return(Args...)>
            {
                using ClassType = void;
                using ReturnType = Return;
                using ArgsTuple = std::tuple<Args...>;

                static constexpr std::index_sequence_for<Args...> IndexSequence {};
                static constexpr bool IsConst = false;
                static constexpr bool IsMember = false;
                static constexpr bool IsFunctor = false;

                static Type ArgType(const std::size_t index) noexcept;
            };

            /** @brief Describes a member function */
            template<typename Class, typename Return, typename ...Args>
            struct FunctionDecomposer<Return(Class::*)(Args...)> : FunctionDecomposer<Return(Args...)>
            {
                using ClassType = Class;

                static constexpr bool IsMember = true;
            };

            /** @brief Describes a const member function */
            template<typename Class, typename Return, typename ...Args>
            struct FunctionDecomposer<Return(Class::*)(Args...) const> : FunctionDecomposer<Return(Class::*)(Args...)>
            {
                static constexpr bool IsConst = true;
            };

            /** @brief Describes an std::function */
            template<typename Type>
            struct FunctionDecomposer<Type, decltype(void(&Type::operator()))> : FunctionDecomposer<decltype(&Type::operator())>
            {
                static constexpr bool IsFunctor = true;
            };

            /** @brief Helper that catch member functions and get their Decomposer */
            template<typename Class, typename Return, typename ...Args>
            constexpr FunctionDecomposer<Return(Class::*)(Args...)> ToFunctionDecomposer(Return(Class::*)(Args...));

            /** @brief Helper that catch const member functions and get their Decomposer */
            template<typename Class, typename Return, typename ...Args>
            constexpr FunctionDecomposer<Return(Class::*)(Args...) const> ToFunctionDecomposer(Return(Class::*)(Args...) const);

            /** @brief Helper that catch free functions and get their Decomposer */
            template<typename Return, typename ...Args>
            constexpr FunctionDecomposer<Return(Args...)> ToFunctionDecomposer(Return(Args...));

            /** @brief Helper that catch functors and get their Decomposer */
            template<typename Type>
            constexpr FunctionDecomposer<Type, decltype(void(&Type::operator()))> ToFunctionDecomposer(Type);

            constexpr void ToFunctionDecomposer(...);

            /** @brief Helper that retreive Decomposer of any function signature */
            template<typename FunctionType>
            using FunctionDecomposerHelper = decltype(ToFunctionDecomposer(std::declval<FunctionType>()));

            /** @brief Opaque function type */
            using OpaqueFunction = const void *;

            /** @brief Helper function used to instantiate a dummy variable for a given function */
            template<auto FunctionPtr>
            struct FunctionIdentifier
            {
                static OpaqueFunction Get(void) noexcept {
                    static const int dummy = 0;
                    return static_cast<OpaqueFunction>(&dummy);
                }
            };

            /** @brief Helper used to retreive a function identifier */
            template<auto FunctionPtr>
            [[nodiscard]] OpaqueFunction GetFunctionIdentifier(void) noexcept { return FunctionIdentifier<FunctionPtr>::Get(); }

            constexpr auto VarSmallOptimizationSize = KF_META_VAR_SMALL_OPTIMIZATION_SIZE;

            /** @brief Helper to know if a type is trivial or not */
            template<typename Type>
            constexpr bool IsVarSmallOptimized = ConstexprTernary((std::is_same_v<Type, void>), false, sizeof(Type) <= VarSmallOptimizationSize);

            /** @brief Helpers to check if an operator is avaible on a Type */
            template<typename Type> using BoolOperatorCheck = decltype(std::declval<Type>().operator bool());
            template<typename Type> using UnaryMinusCheck = decltype(- std::declval<Type>());
            template<typename Type> using BinaryAdditionCheck = decltype(std::declval<Type>() + std::declval<Type>());
            template<typename Type> using BinarySubstractionCheck = decltype(std::declval<Type>() - std::declval<Type>());
            template<typename Type> using BinaryMultiplicationCheck = decltype(std::declval<Type>() * std::declval<Type>());
            template<typename Type> using BinaryDivisionCheck = decltype(std::declval<Type>() / std::declval<Type>());
            template<typename Type> using BinaryModuloCheck = decltype(std::declval<Type>() % std::declval<Type>());
            template<typename Type> using AssignmentAdditionCheck = decltype(std::declval<Type&>() += std::declval<Type>());
            template<typename Type> using AssignmentSubstractionCheck = decltype(std::declval<Type&>() -= std::declval<Type>());
            template<typename Type> using AssignmentMultiplicationCheck = decltype(std::declval<Type&>() *= std::declval<Type>());
            template<typename Type> using AssignmentDivisionCheck = decltype(std::declval<Type&>() /= std::declval<Type>());
            template<typename Type> using AssignmentModuloCheck = decltype(std::declval<Type&>() %= std::declval<Type>());

            /** Helpers used to generate opaque default, copy and move constructor functions */
            template<typename Type>
            void MakeDefaultConstructor(void *instance) noexcept_constructible(Type);
            template<typename Type>
            void MakeCopyConstructor(void *instance, const void *data) noexcept_copy_constructible(Type);
            template<typename Type>
            void MakeMoveConstructor(void *instance, void *data) noexcept_move_constructible(Type);

            /** @brief Helpers used to generate opaque assignment functions */
            template<typename Type>
            void MakeCopyAssignment(void *instance, const void *data) noexcept_copy_assignable(Type);
            template<typename Type>
            void MakeMoveAssignment(void *instance, void *data) noexcept_move_assignable(Type);

            /** @brief Helper used to generate opaque destructor functions */
            template<typename Type>
            void MakeDestructor(void *data) noexcept_destructible(Type);

            /** @brief Helpers used to generate opaque primitive functions */
            template<typename Type, std::enable_if_t<std::is_convertible_v<Type, bool>>* = nullptr>
            [[nodiscard]] bool MakeToBool(const void *data) noexcept_convertible(Type, bool) { return static_cast<bool>(*reinterpret_cast<const Type *>(data)); }
            template<typename Type, std::enable_if_t<!std::is_convertible_v<Type, bool>>* = nullptr>
            [[nodiscard]] bool MakeToBool(const void *data) noexcept_expr(std::declval<Type>().operator bool()) { return reinterpret_cast<const Type *>(data)->operator bool(); }

            /** @brief Helpers used to generate opaque operators functions */
            template<typename Type, auto OperatorFunc, UnaryOperator Operator>
            Var MakeUnaryOperator(const void *data);
            template<typename Type, auto OperatorFunc, BinaryOperator Operator>
            Var MakeBinaryOperator(const void *data, const Var &var);
            template<typename Type, auto OperatorFunc, AssignmentOperator Operator>
            void MakeAssignmentOperator(void *data, const Var &var);

            /** @brief Helpers to generate unary operator functions */
            template<typename Type>
            [[nodiscard]] Type UnaryMinus(const Type &var) noexcept_expr(-var) { return -var; }

            /** @brief Helpers to generate binary operator functions */
            template<typename Type>
            [[nodiscard]] Type BinaryAddition(const Type &lhs, const Type &rhs) noexcept_expr(lhs + rhs) { return Type(lhs + rhs); }
            template<typename Type>
            [[nodiscard]] Type BinarySubstraction(const Type &lhs, const Type &rhs) noexcept_expr(lhs - rhs) { return Type(lhs - rhs); }
            template<typename Type>
            [[nodiscard]] Type BinaryMultiplication(const Type &lhs, const Type &rhs) noexcept_expr(lhs * rhs) { return Type(lhs * rhs); }
            template<typename Type>
            [[nodiscard]] Type BinaryDivision(const Type &lhs, const Type &rhs) noexcept_expr(lhs / rhs) { return Type(lhs / rhs); }
            template<typename Type, std::enable_if_t<!std::is_floating_point_v<Type>>* = nullptr>
            [[nodiscard]] Type BinaryModulo(const Type &lhs, const Type &rhs) noexcept_expr(lhs % rhs) { return Type(lhs % rhs); }
            template<typename Type, std::enable_if_t<std::is_floating_point_v<Type>>* = nullptr>
            [[nodiscard]] Type BinaryModulo(const Type &lhs, const Type &rhs) noexcept { return static_cast<Type>(static_cast<std::int64_t>(lhs) % static_cast<std::int64_t>(rhs)); }

            /** @brief Helpers to generate assignment operator functions */
            template<typename Type>
            void AssignmentAddition(Type &lhs, const Type &rhs) noexcept_expr(lhs += rhs) { lhs += rhs; }
            template<typename Type>
            void AssignmentSubstraction(Type &lhs, const Type &rhs) noexcept_expr(lhs -= rhs) { lhs -= rhs; }
            template<typename Type>
            void AssignmentMultiplication(Type &lhs, const Type &rhs) noexcept_expr(lhs *= rhs) { lhs *= rhs; }
            template<typename Type>
            void AssignmentDivision(Type &lhs, const Type &rhs) noexcept_expr(lhs /= rhs) { lhs /= rhs; }
            template<typename Type, std::enable_if_t<!std::is_floating_point_v<Type>>* = nullptr>
            void AssignmentModulo(Type &lhs, const Type &rhs) noexcept_expr(lhs %= rhs) { lhs %= rhs; }
            template<typename Type, std::enable_if_t<std::is_floating_point_v<Type>>* = nullptr>
            void AssignmentModulo(Type &lhs, const Type &rhs) noexcept_expr(lhs = BinaryModulo(lhs, rhs)) { lhs = BinaryModulo(lhs, rhs); }

            /** @brief Helpers to generate pointer operators functions */
            template<typename Type>
            [[nodiscard]] Type BinaryAdditionPointer(const Type lhs, const std::size_t rhs) noexcept { return lhs + rhs; }
            template<typename Type>
            [[nodiscard]] Type BinarySubstractionPointer(const Type lhs, const std::size_t rhs) noexcept { return lhs - rhs; }
            template<typename Type>
            void AssignmentAdditionPointer(Type &lhs, const std::size_t rhs) noexcept { lhs += rhs; }
            template<typename Type>
            void AssignmentSubstractionPointer(Type &lhs, const std::size_t rhs) noexcept { lhs -= rhs; }

            /** @brief Helper used to forward an argument of an invoked function */
            template<typename ArgType, bool AllowImplicitMove>
            decltype(auto) ForwardArgument(Var *any);

            /** @brief Meta function invoker
             * Will perform different semantics uppon function's arguments
             * RVakue - Perfect forwarding, will move anyway (even references) !
             * LValue volatile - Perfect forwarding
             * LValue constant - Forward if possible, else try to convert
             * Value - Forward if possible, else try to convert
             */
            template<typename Type, auto FunctionPtr, bool AllowImplicitMove, typename Decomposer, std::size_t ...Indexes>
            Var Invoke([[maybe_unused]] const void *instance, Var *args, const std::index_sequence<Indexes...> &sequence);

            /** @brief Meta functor invoker */
            template<typename Type, bool AllowImplicitMove, typename Decomposer, typename Functor, std::size_t ...Indexes>
            Var Invoke(Functor &functor, [[maybe_unused]] const void *instance, Var *args, const std::index_sequence<Indexes...> &sequence);

            /** @brief Simple structure that holds a type */
            template<typename Target>
            struct TypeHolder { using Type = Target; };

            /** @brief Helper used to format specific types to other */
            template<typename Input>
            struct ArrangeType
            {
                using PointerType = decltype([] {
                    if constexpr (std::is_array_v<Input>)
                        return static_cast<std::remove_extent_t<Input> **>(nullptr);
                    else
                        return static_cast<Input*>(nullptr);
                }());

                using Type = std::remove_pointer_t<PointerType>;
            };

            /** @brief Helper used to retreive a void pointer from a reference (either a reference to nullptr or a variable) */
            template<typename Type>
            [[nodiscard]] static constexpr const void *RetreiveOpaquePtr(const Type &input) noexcept
            {
                if constexpr (std::is_same_v<std::remove_cvref_t<Type>, std::nullptr_t>)
                    return static_cast<const void *>(input);
                else if constexpr (std::is_pointer_v<std::remove_cvref_t<Type>>)
                    return static_cast<const void *>(input);
                else
                    return static_cast<const void *>(&input);
            };
        }
    }
}