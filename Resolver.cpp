/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Resolver
 */

#include "Meta.hpp"

using namespace kF;

void Meta::Resolver::RegisterMetaType(Type type) noexcept_ndebug
{
    kFAssert(!FindTypeByName(type.typeID()).operator bool(),
        throw std::logic_error("Meta::Resolver::RegisterMetaTypeDescriptor: Type already registered"));
    GetDescriptors().emplace_back(type);
}

Meta::Type Meta::Resolver::FindTypeByName(const Type::TypeID id) noexcept
{
    for (const auto type : GetDescriptors())
        if (type.typeID() == id)
            return type;
    return Type();
}

Meta::Type Meta::Resolver::FindTypeByName(const HashedName name) noexcept
{
    for (const auto type : GetDescriptors())
        if (type.name() == name)
            return type;
    return Type();
}

void Meta::Resolver::Clear(void) noexcept
{
    for (Type &descriptor : GetDescriptors()) {
        descriptor.clear();
    }
    GetDescriptors().clear();
}