/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Resolver
 */

inline void kF::Meta::Resolver::RegisterMetaType(Type type) noexcept_ndebug
{
    kFAssert(!FindType(type.typeID()).operator bool(),
        throw std::logic_error("Meta::Resolver::RegisterMetaTypeDescriptor: Type already registered"));
    GetDescriptors().emplace_back(type);
}

inline kF::Meta::Type kF::Meta::Resolver::FindType(const Type::TypeID id) noexcept
{
    for (const auto type : GetDescriptors())
        if (type.typeID() == id)
            return type;
    return Type();
}

inline kF::Meta::Type kF::Meta::Resolver::FindType(const HashedName name) noexcept
{
    for (const auto type : GetDescriptors())
        if (type.name() == name)
            return type;
    return Type();
}

inline void kF::Meta::Resolver::Clear(void) noexcept
{
    for (Type &descriptor : GetDescriptors()) {
        descriptor.clear();
    }
    GetDescriptors().clear();
}