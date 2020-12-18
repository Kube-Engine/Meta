/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Resolver
 */

inline void kF::Meta::Resolver::RegisterMetaType(const Type type) noexcept_ndebug
{
    kFAssert(!FindType(type.typeID()).operator bool(),
        throw std::logic_error("Meta::Resolver::RegisterMetaTypeDescriptor: Type already registered"));
    _Cache.types.push(type);
}

inline void kF::Meta::Resolver::RegisterMetaTemplateSpecialization(const HashedName name, const Type specialization) noexcept_ndebug
{
    if (const auto descriptor = const_cast<TemplateDescriptor *>(FindTemplate(name)); descriptor)
        descriptor->specializations.push(specialization);
    else
        _Cache.templates.push(TemplateDescriptor {
            name: name,
            specializations: { specialization }
        });
}

inline kF::Meta::Type kF::Meta::Resolver::FindType(const Type::TypeID id) noexcept
{
    for (const auto type : _Cache.types)
        if (type.typeID() == id) [[unlikely]]
            return type;
    return Type();
}

inline kF::Meta::Type kF::Meta::Resolver::FindType(const HashedName name) noexcept
{
    for (const auto type : _Cache.types)
        if (type.name() == name) [[unlikely]]
            return type;
    return Type();
}

inline const kF::Meta::Resolver::TemplateDescriptor *kF::Meta::Resolver::FindTemplate(const HashedName name) noexcept
{
    for (const auto &descriptor : _Cache.templates) {
        if (descriptor.name == name) [[unlikely]]
            return &descriptor;
    }
    return nullptr;
}

inline kF::Meta::Type kF::Meta::Resolver::FindTemplateSpecialization(const HashedName name, const HashedName specializationName) noexcept
{
    const auto descriptor = FindTemplate(name);
    if (!descriptor)
        return Type();
    return FindTemplateSpecialization(descriptor, specializationName);
}

inline kF::Meta::Type kF::Meta::Resolver::FindTemplateSpecialization(const TemplateDescriptor *descriptor, const HashedName specializationName) noexcept
{
    for (const auto &type : descriptor->specializations) {
        if (type.name() == specializationName) [[unlikely]]
            return type;
    }
    return Type();
}

inline void kF::Meta::Resolver::Clear(void) noexcept
{
    for (auto &descriptor : _Cache.types) {
        descriptor.clear();
    }
    for (auto &descriptor : _Cache.templates) {
        for (auto &specialization : descriptor.specializations)
            specialization.clear();
    }
    _Cache.types.clear();
    _Cache.templates.clear();
}