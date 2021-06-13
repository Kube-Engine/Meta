/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Meta Data
 */

#pragma once

#include <Kube/Core/TrivialFunctor.hpp>

#include "Type.hpp"
#include "Signal.hpp"

/**
 * @brief Data is used to store meta-data about a getter and a setter used like a property
 */
class kF::Meta::Data
{
public:
    using GetFunc = Core::TrivialFunctor<Var(const void *)>;
    using SetCopyFunc = Core::TrivialFunctor<Var(const void *, const Var &)>;
    using SetMoveFunc = Core::TrivialFunctor<Var(const void *, Var &&)>;

    /** @brief Describe a meta data */
    struct alignas_cacheline Descriptor
    {
        const HashedName name {};
        const bool isStatic {};
        const Type type {};
        const GetFunc getFunc {};
        const SetCopyFunc setCopyFunc {};
        const SetMoveFunc setMoveFunc {};

        /** @brief Construct a Descriptor */
        template<typename Type, auto GetFunctionPtr, auto SetCopyFunctionPtr, auto SetMoveFunctionPtr>
        [[nodiscard]] static Descriptor Construct(const HashedName name) noexcept;
    };

    static_assert_fit_cacheline(Descriptor);

    /** @brief Construct passing a descriptor instance */
    Data(const Descriptor *desc = nullptr) noexcept : _desc(desc) {}

    /** @brief Copy constructor */
    Data(const Data &other) noexcept = default;

    /** @brief Copy assignment */
    Data &operator=(const Data &other) noexcept = default;

    /** @brief Fast valid check */
    [[nodiscard]] operator bool(void) const noexcept { return _desc; }

    /** @brief Compare operator */
    [[nodiscard]] bool operator==(const Data &other) const noexcept { return _desc == other._desc; }
    [[nodiscard]] bool operator!=(const Data &other) const noexcept { return _desc != other._desc; }


    /** @brief Get data's hashed name */
    [[nodiscard]] HashedName name(void) const noexcept { return _desc->name; }

    /** @brief Check if the data is static */
    [[nodiscard]] bool isStatic(void) const noexcept { return _desc->isStatic; }

    /** @brief Check if the variable is read-only */
    [[nodiscard]] bool isReadOnly(void) const noexcept { return !_desc->setCopyFunc && !_desc->setMoveFunc; }

    /** @brief Check if the variable is copy settable */
    [[nodiscard]] bool isCopySettable(void) const noexcept { return _desc->setCopyFunc; }

    /** @brief Check if the variable is move settable */
    [[nodiscard]] bool isMoveSettable(void) const noexcept { return _desc->setMoveFunc; }

    /** @brief Get underlying data type */
    [[nodiscard]] Type type(void) const noexcept { return _desc->type; }


    /** @brief Get the underlying instance */
    [[nodiscard]] Var get(const Var &instance) const { return _desc->getFunc(instance.data()); }
    [[nodiscard]] Var get(const void *instance = nullptr) const { return _desc->getFunc(instance); }

    /** @brief Call member setter using a opaque variable */
    template<typename Type>
    [[nodiscard]] Var set(const Var &instance, Type &&value) const { return set(instance.data(), std::forward<Type>(value)); }

    /** @brief Call member setter using a opaque pointer */
    template<typename Type>
    [[nodiscard]] Var set(const void *instance, Type &&value) const;

    /** @brief Call static setter */
    template<typename Type>
    [[nodiscard]] Var set(Type &&value) const { return set(nullptr, std::forward<Type>(value)); }

private:
    const Descriptor *_desc = nullptr;
};