/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Meta Resolver
 */

#pragma once

#include <Kube/Core/Vector.hpp>
#include <Kube/Core/FlatVector.hpp>

#include "Type.hpp"

/**
 * @brief Resolver is used to store and retreive meta-data at runtime
 */
class kF::Meta::Resolver
{
public:
    /** @brief Describe a template class */
    struct alignas_quarter_cacheline TemplateDescriptor
    {
        HashedName name {};
        Core::FlatVector<Type> specializations;
    };

    /** @brief Cache stored in static memory */
    struct alignas_cacheline Cache
    {
        Core::Vector<Type> types;
        Core::Vector<TemplateDescriptor> templates;
    };

    /** @brief Register a new type into the resolver */
    static void RegisterMetaType(const Type type) noexcept_ndebug;

    /** @brief Register a new type into the resolver */
    static void RegisterMetaTemplateSpecialization(const HashedName name, const Type specialization) noexcept_ndebug;


    /** @brief Resolve a type with its ID */
    [[nodiscard]] static Type FindType(const Type::TypeID id) noexcept;

    /** @brief Resolve a type with its name */
    [[nodiscard]] static Type FindType(const HashedName name) noexcept;


    /** @brief Resolve a template type with its name */
    [[nodiscard]] static const TemplateDescriptor *FindTemplate(const HashedName name) noexcept;

    /** @brief Resolve a template specialization with its name and specialization name */
    [[nodiscard]] static Type FindTemplateSpecialization(const HashedName name, const HashedName specializationName) noexcept;

    /** @brief Resolve a template specialization with its descriptor and specialization name */
    [[nodiscard]] static Type FindTemplateSpecialization(const TemplateDescriptor *descriptor, const HashedName specializationName) noexcept;


    /** @brief Clear all stored types */
    static void Clear(void) noexcept;

private:
    static inline Cache _Cache {};

    /** @brief Resolver is a singleton */
    Resolver(void) = delete;
};