/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Meta Resolver
 */

#pragma once

#include "Type.hpp"

/**
 * @brief Resolver is used to store and retreive meta-data at runtime
 */
class kF::Meta::Resolver
{
public:
    using Descriptors = std::vector<Type>;

    /** @brief Register a new type into the resolver */
    static void RegisterMetaType(Type type) noexcept_ndebug;

    /** @brief Resolve a type with its ID */
    [[nodiscard]] static Type FindType(const Type::TypeID id) noexcept;

    /** @brief Resolve a type with its name */
    [[nodiscard]] static Type FindType(const HashedName name) noexcept;

    /** @brief Clear all stored types */
    static void Clear(void) noexcept;

private:
    /** @brief Resolver is a singleton */
    Resolver(void) = delete;

    /** @brief Get internal list of descriptors */
    static Descriptors &GetDescriptors(void) {
        static Descriptors data;
        return data;
    }
};