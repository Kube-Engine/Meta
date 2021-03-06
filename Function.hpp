/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Meta Function
 */

#pragma once

#include "Type.hpp"

/**
 * @brief Function is used to store meta-data about a free / member / static function
 */
class kF::Meta::Function
{
public:
    using InvokeFunc = Core::TrivialFunctor<Var(const void *, Var *), Core::CacheLineEighthSize * 3>;
    using ArgTypeFunc = Type(*)(const std::size_t) noexcept;

    struct alignas_cacheline Descriptor
    {
        const HashedName name { 0u };
        const bool isStatic { false };
        const bool isConst { false };
        const std::size_t argsCount { 0u };
        const Type returnType {};
        const ArgTypeFunc argTypeFunc { nullptr };
        const InvokeFunc invokeFunc {};

        template<typename Type, auto FunctionPtr>
        [[nodiscard]] static Descriptor Construct(const HashedName name) noexcept;
    };

    static_assert_fit_cacheline(Descriptor);

    /** @brief Construct passing a descriptor instance */
    Function(const Descriptor *desc = nullptr) noexcept : _desc(desc) {}

    /** @brief Copy constructor */
    Function(const Function &other) noexcept = default;

    /** @brief Copy assignment */
    Function &operator=(const Function &other) noexcept = default;

    /** @brief Fast valid check */
    [[nodiscard]] operator bool(void) const noexcept { return _desc; }

    /** @brief Compare operator */
    [[nodiscard]] bool operator==(const Function &other) const noexcept { return _desc == other._desc; }
    [[nodiscard]] bool operator!=(const Function &other) const noexcept { return _desc != other._desc; }

    /** @brief Retreive function's name */
    [[nodiscard]] HashedName name(void) const noexcept { return _desc->name; }

    /** @brief Retreive arguments count */
    [[nodiscard]] std::size_t argsCount(void) const noexcept { return _desc->argsCount; }

    /** @brief Retreive an argument type */
    [[nodiscard]] Type argType(const std::size_t index) const noexcept { return _desc->argTypeFunc(index); }

    /** @brief Check if the underlying function is static */
    [[nodiscard]] bool isStatic(void) const noexcept { return _desc->isStatic; }

    /** @brief Check if the underlying function is a const-member */
    [[nodiscard]] bool isConst(void) const noexcept { return _desc->isConst; }

    /** @brief Invoke a member function using a var instance */
    template<typename ...Args>
    [[nodiscard]] Var invoke(const Var &instance, Args &&...args) const { return invoke(instance.data(), std::forward<Args>(args)...); }

    /** @brief Invoke a member function */
    template<typename ...Args>
    [[nodiscard]] Var invoke(const void *instance, Args &&...args) const;

    /** @brief Invoke a static function */
    template<typename ...Args>
    [[nodiscard]] Var invoke(Args &&...args) const { return invoke(nullptr, std::forward<Args>(args)...); }

private:
    const Descriptor *_desc = nullptr;
};