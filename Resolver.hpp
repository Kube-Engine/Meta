/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Meta Resolver
 */

#pragma once

#include <Kube/Core/Vector.hpp>

#include "Type.hpp"

/**
 * @brief Resolver is used to store and retreive meta-data at runtime
 */
class kF::Meta::Resolver
{
public:
    struct TemplateDescriptor
    {
        HashedName name { 0u };
    };

    using Descriptors = Core::Vector<Type>;
    using TemplateDescriptors = Core::Vector<TemplateDescriptor>;

    /** @brief Register a new type into the resolver */
    static void RegisterMetaType(Type type) noexcept_ndebug;

    /** @brief Register a new type into the resolver */
    static void RegisterTemplateType(const HashedName name) noexcept_ndebug;


    /** @brief Resolve a type with its ID */
    [[nodiscard]] static Type FindType(const Type::TypeID id) noexcept;

    /** @brief Resolve a type with its name */
    [[nodiscard]] static Type FindType(const HashedName name) noexcept;


    /** @brief Resolve a type with its name */
    [[nodiscard]] static bool TemplateExists(const HashedName name) noexcept;


    /** @brief Clear all stored types */
    static void Clear(void) noexcept;

private:
    static inline Descriptors _Descriptors {};
    static inline TemplateDescriptors _TemplateDescriptors {};

    /** @brief Resolver is a singleton */
    Resolver(void) = delete;
};