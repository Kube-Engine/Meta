/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Meta Data
 */

template<typename Type, auto GetFunctionPtr, auto SetFunctionPtr>
kF::Meta::Data::Descriptor kF::Meta::Data::Descriptor::Construct(const HashedName name) noexcept
{
    using GetFunctionType = decltype(GetFunctionPtr);
    using GetDecomposer = Internal::FunctionDecomposerHelper<GetFunctionType>;
    using SetFunctionType = decltype(SetFunctionPtr);
    using SetDecomposer = Internal::FunctionDecomposerHelper<SetFunctionType>;

    static_assert(GetFunctionPtr != nullptr, "A meta data must at least have a getter");

    return Descriptor {
        .name = name,
        .isStatic = GetDecomposer::IsFunctor || !GetDecomposer::IsMember,
        .type = Factory<typename GetDecomposer::ReturnType>::Resolve(),
        .getFunc = [](const void *instance) -> Var {
            if constexpr (GetDecomposer::IsFunctor)
                return Var::Assign(GetFunctionPtr());
            else if constexpr (GetDecomposer::IsMember)
                return Var::Assign(((const_cast<Type *>(reinterpret_cast<const Type *>(instance)))->*GetFunctionPtr)());
            else
                return Var::Assign((*GetFunctionPtr)());
        },
        .setFunc = ConstexprTernary(
            SetFunctionPtr,
            ([]([[maybe_unused]] const void *instance, Var &value) -> Var {
                return Internal::Invoke<Type, SetFunctionPtr, SetDecomposer>(instance, &value, SetDecomposer::IndexSequence);
            }),
            nullptr
        )
    };
}

template<typename Type>
kF::Var kF::Meta::Data::set(const void *instance, Type &&value) const
{
    kFAssert(!isReadOnly(),
        throw std::runtime_error("Meta::Data::set: Data is read only"));
    if constexpr (std::is_same_v<std::remove_cvref_t<decltype(value)>, Var>) {
        if constexpr (std::is_const_v<std::remove_reference_t<decltype(value)>>)
            return (*_desc->setFunc)(instance, const_cast<Var &>(value));
        else
            return (*_desc->setFunc)(instance, value);
    } else {
        auto var = kF::Var::Assign(std::forward<Type>(value));
        return (*_desc->setFunc)(instance, var);
    }
}