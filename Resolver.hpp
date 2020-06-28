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

    static void RegisterMetaType(Type type) noexcept_ndebug;

    [[nodiscard]] static Type FindTypeByName(const Type::TypeID id) noexcept;

    [[nodiscard]] static Type FindTypeByName(const HashedName name) noexcept;

    static void Clear(void) noexcept;

private:
    Resolver(void) = delete;

    static Descriptors &GetDescriptors(void) {
        static Descriptors data;
        return data;
    }
};