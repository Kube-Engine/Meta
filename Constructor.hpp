/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Meta custom constructor
 */

#pragma once

#include "Type.hpp"

class kF::Meta::Constructor
{
public:
    using InvokeFunc = bool(*)(void *, Var *);
    using ArgTypeFunc = Type(*)(const std::size_t) noexcept;

    struct KF_ALIGN_HALF_CACHELINE Descriptor
    {
        const std::size_t argsCount;
        const ArgTypeFunc argTypeFunc;
        const Type type;
        const InvokeFunc invokeFunc;

        template<typename Type, typename ...Args> requires std::constructible_from<Type, Args...>
        static Descriptor Construct(void) noexcept;
    };

    static_assert(sizeof(Descriptor) == 32, "Constructor descriptor take 32 bytes");

    /** @brief Construct passing a descriptor instance */
    Constructor(const Descriptor *desc = nullptr) noexcept : _desc(desc) {}

    /** @brief Copy constructor */
    Constructor(const Constructor &other) noexcept = default;

    /** @brief Copy assignment */
    Constructor &operator=(const Constructor &other) noexcept = default;

    /** @brief Fast valid check */
    [[nodiscard]] operator bool(void) const noexcept { return _desc; }

    /** @brief Compare operator */
    [[nodiscard]] bool operator==(const Constructor &other) const noexcept { return _desc == other._desc; }
    [[nodiscard]] bool operator!=(const Constructor &other) const noexcept { return _desc != other._desc; }

    /** @brief Retreive arguments count */
    [[nodiscard]] std::size_t argsCount(void) const noexcept { return _desc->argsCount; }

    /** @brief Retreive an argument type */
    [[nodiscard]] Type argType(const std::size_t index) const noexcept { return _desc->argTypeFunc(index); }

    /** @brief Underlying constructor type */
    [[nodiscard]] Type type(void) const noexcept { return _desc->type; }

    /** @brief Invoke a constructor into a var instance */
    template<typename ...Args>
    [[nodiscard]] Var invoke(Args &&...args) const;

    /** @brief Invoke a constructor on the given instance  */
    template<typename ...Args>
    [[nodiscard]] bool invoke(void *instance, Args &&...args) const;

private:
    const Descriptor *_desc = nullptr;
};