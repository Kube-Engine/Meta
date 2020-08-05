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

#include "Forward.hpp"

/** @brief Compile-time ternary expression */
#define ConstexprTernary(condition, body, elseBody) [] { if constexpr (condition) { return body; } else { return elseBody; } }()

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
                using ReturnType = std::remove_cvref_t<Return>;
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

            constexpr auto TrivialTypeSizeLimit = sizeof(std::string);

            /** @brief Helper to know if a type is trivial or not */
            template<typename Type>
            constexpr bool IsTrivial = sizeof(Type) <= TrivialTypeSizeLimit;

            /** @brief std::enable_if alias for trivial types */
            template<typename Type, typename Internal = void>
            using EnableIfTrivial = std::enable_if<IsTrivial<Type>, Internal>;

            /** @brief std::enable_if alias for non-trivial types */
            template<typename Type, typename Internal = void>
            using EnableIfNotTrivial = std::enable_if<!IsTrivial<Type>>;

            /** Helpers used to generate opaque default, copy and move constructor functions */
            template<typename Type>
            void MakeDefaultConstructor(void *instance);
            template<typename Type>
            void MakeCopyConstructor(void *instance, const void *data);
            template<typename Type>
            void MakeMoveConstructor(void *instance, void *data);

            /** @brief Helpers used to generate opaque assignment functions */
            template<typename Type>
            void MakeCopyAssignment(void *instance, const void *data);
            template<typename Type>
            void MakeMoveAssignment(void *instance, void *data);

            /** @brief Helper used to generate opaque destructor functions */
            template<typename Type>
            void MakeDestructor(void *data);

            /** @brief Helpers used to generate opaque conversion functions */
            template<typename From, typename To, typename = std::enable_if<std::is_convertible_v<From, To>>>
            Var MakeConverter(const void *from);
            template<typename From, typename To, auto FunctionPtr>
            Var MakeCustomConverter(const void *from);

            /** @brief Helpers used to generate opaque primitive functions */
            template<typename Type>
            [[nodiscard]] bool MakeToBool(const void *data);

            /** @brief Helpers used to generate opaque operators functions */
            template<typename Type, auto OperatorFunc, kF::Meta::UnaryOperator Operator>
            Var MakeUnaryOperator(const void *data);
            template<typename Type, auto OperatorFunc, kF::Meta::BinaryOperator Operator>
            Var MakeBinaryOperator(const void *data, const Var &var);
            template<typename Type, auto OperatorFunc, kF::Meta::AssignmentOperator Operator>
            void MakeAssignmentOperator(void *data, const Var &var);

            /** @brief Helpers to generate unary operator functions */
            template<typename Type>
            [[nodiscard]] Type UnaryMinus(const Type &var) { return -var; }

            /** @brief Helpers to generate binary operator functions */
            template<typename Type>
            [[nodiscard]] Type BinaryAddition(const Type &lhs, const Type &rhs) { return Type(lhs + rhs); }
            template<typename Type>
            [[nodiscard]] Type BinarySubstraction(const Type &lhs, const Type &rhs) { return Type(lhs - rhs); }
            template<typename Type>
            [[nodiscard]] Type BinaryMultiplication(const Type &lhs, const Type &rhs) { return Type(lhs * rhs); }
            template<typename Type>
            [[nodiscard]] Type BinaryDivision(const Type &lhs, const Type &rhs) { return Type(lhs / rhs); }
            template<typename Type, std::enable_if_t<!std::is_floating_point_v<Type>>* = nullptr>
            [[nodiscard]] Type BinaryModulo(const Type &lhs, const Type &rhs) { return Type(lhs % rhs); }
            template<typename Type, std::enable_if_t<std::is_floating_point_v<Type>>* = nullptr>
            [[nodiscard]] Type BinaryModulo(const Type &lhs, const Type &rhs) { return static_cast<Type>(static_cast<std::int64_t>(lhs) % static_cast<std::int64_t>(rhs)); }

            /** @brief Helpers to generate assignment operator functions */
            template<typename Type>
            void AssignmentAddition(Type &lhs, const Type &rhs) { lhs += rhs; }
            template<typename Type>
            void AssignmentSubstraction(Type &lhs, const Type &rhs) { lhs -= rhs; }
            template<typename Type>
            void AssignmentMultiplication(Type &lhs, const Type &rhs) { lhs *= rhs; }
            template<typename Type>
            void AssignmentDivision(Type &lhs, const Type &rhs) { lhs /= rhs; }
            template<typename Type, std::enable_if_t<!std::is_floating_point_v<Type>>* = nullptr>
            void AssignmentModulo(Type &lhs, const Type &rhs) { lhs %= rhs; }
            template<typename Type, std::enable_if_t<std::is_floating_point_v<Type>>* = nullptr>
            void AssignmentModulo(Type &lhs, const Type &rhs) { lhs = BinaryModulo(lhs, rhs); }

            /** @brief Helpers to generate pointer operators functions */
            template<typename Type>
            [[nodiscard]] Type BinaryAdditionPointer(const Type lhs, const std::size_t rhs) { return lhs + rhs; }
            template<typename Type>
            [[nodiscard]] Type BinarySubstractionPointer(const Type lhs, const std::size_t rhs) { return lhs - rhs; }
            template<typename Type>
            void AssignmentAdditionPointer(Type &lhs, const std::size_t rhs) { lhs += rhs; }
            template<typename Type>
            void AssignmentSubstractionPointer(Type &lhs, const std::size_t rhs) { lhs -= rhs; }

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

            /** @brief Meta function invoker
             * Will perform different semantics uppon function's arguments
             * RVakue - Perfect forwarding, will move anyway (even references) !
             * LValue volatile - Perfect forwarding
             * LValue constant - Forward if possible, else try to convert
             * Value - Forward if possible, else try to convert
             */
            template<typename Type, auto FunctionPtr, typename Decomposer, std::size_t ...Indexes>
            Var Invoke([[maybe_unused]] const void *instance, Var *args, const std::index_sequence<Indexes...> &sequence);

            /** @brief Meta functor invoker */
            template<typename Type, typename Decomposer, typename Functor, std::size_t ...Indexes>
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
            static constexpr const void *RetreiveOpaquePtr(const Type &input) {
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