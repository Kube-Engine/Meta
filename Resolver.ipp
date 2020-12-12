/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Resolver
 */

inline void kF::Meta::Resolver::RegisterMetaType(Type type) noexcept_ndebug
{
    kFAssert(!FindType(type.typeID()).operator bool(),
        throw std::logic_error("Meta::Resolver::RegisterMetaTypeDescriptor: Type already registered"));
    _Descriptors.push(type);
}

inline void kF::Meta::Resolver::RegisterTemplateType(const HashedName name) noexcept_ndebug
{
    if (!TemplateExists(name))
        _TemplateDescriptors.push(name);
}

inline kF::Meta::Type kF::Meta::Resolver::FindType(const Type::TypeID id) noexcept
{
    for (const auto type : _Descriptors)
        if (type.typeID() == id)
            return type;
    return Type();
}

inline kF::Meta::Type kF::Meta::Resolver::FindType(const HashedName name) noexcept
{
    for (const auto type : _Descriptors)
        if (type.name() == name)
            return type;
    return Type();
}

inline bool kF::Meta::Resolver::TemplateExists(const HashedName name) noexcept
{
    for (const auto it : _TemplateDescriptors)
        if (it.name == name)
            return true;
    return false;
}

inline void kF::Meta::Resolver::Clear(void) noexcept
{
    for (auto &descriptor : _Descriptors) {
        descriptor.clear();
    }
    _Descriptors.clear();
    _TemplateDescriptors.clear();
}