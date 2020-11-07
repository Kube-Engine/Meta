/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Meta Data
 */

#pragma once

#include "Type.hpp"
#include "Signal.hpp"

/**
 * @brief Data is used to store meta-data about a getter and a setter used like a property
 */
class kF::Meta::Data
{
public:
    using GetFunc = Var(*)(const void *);
    using SetFunc = Var(*)(const void *, Var &);

    /** @brief Describe a meta data */
    struct KF_ALIGN_CACHELINE Descriptor
    {
        const HashedName name;
        const bool isStatic;
        const bool isMoveOnly;
        const char _padding[2];
        const Type type;
        const GetFunc getFunc;
        const SetFunc setFunc;
        const Signal signal;

        /** @brief Construct a Descriptor */
        template<typename Type, auto GetFunctionPtr, auto SetFunctionPtr>
        static Descriptor Construct(const HashedName name, const Signal signal = Signal()) noexcept;
    };

    static_assert(sizeof(Descriptor) == Core::CacheLineSize, "Data descriptor must take size of a cacheline");

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

    /** @brief Get underlying data type */
    [[nodiscard]] Type type(void) const noexcept { return _desc->type; }

    /** @brief Check if the variable is read-only */
    [[nodiscard]] bool isReadOnly(void) const noexcept { return !_desc->setFunc; }

    /** @brief Get the underlying instance */
    [[nodiscard]] Var get(const Var &instance) const { return (*_desc->getFunc)(instance.data()); }
    [[nodiscard]] Var get(const void *instance = nullptr) const { return (*_desc->getFunc)(instance); }

    /** @brief Set the underlying instance */
    template<typename Type>
    [[nodiscard]] Var set(const Var &instance, Type &&value) const { return set(instance.data(), std::forward<Type>(value)); }
    template<typename Type>
    [[nodiscard]] Var set(Type &&value) const { return set(nullptr, std::forward<Type>(value)); }
    template<typename Type>
    [[nodiscard]] Var set(const void *instance, Type &&value) const;

private:
    const Descriptor *_desc = nullptr;
};