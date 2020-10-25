/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Meta Data
 */

template<typename Type, auto GetFunctionPtr, auto SetFunctionPtr>
inline kF::Meta::Data::Descriptor kF::Meta::Data::Descriptor::Construct(const HashedName name, const Signal signal) noexcept
{
    using GetFunctionType = decltype(GetFunctionPtr);
    using GetDecomposer = Internal::FunctionDecomposerHelper<GetFunctionType>;
    using SetFunctionType = decltype(SetFunctionPtr);
    using SetDecomposer = Internal::FunctionDecomposerHelper<SetFunctionType>;

    static_assert(GetFunctionPtr != nullptr, "A meta data must at least have a getter");
    static_assert(std::tuple_size_v<typename GetDecomposer::ArgsTuple> == 0, "A meta data must have a getter that take no argument");

    using FlatGetterReturnType = std::remove_cvref_t<typename Internal::FunctionDecomposerHelper<GetFunctionType>::ReturnType>;

    if constexpr (SetFunctionPtr) {
        static_assert(std::tuple_size_v<typename SetDecomposer::ArgsTuple> == 1, "A meta data must have a setter that take only one argument");
        using FlatSetterArg = std::remove_cvref_t<std::tuple_element_t<0, typename SetDecomposer::ArgsTuple>>;
        static_assert(std::is_same_v<FlatSetterArg, FlatGetterReturnType>, "A meta data must have a setter that takes the data type in parameter");
    }

    return Descriptor {
        name: name,
        isStatic: GetDecomposer::IsFunctor || !GetDecomposer::IsMember,
        isMoveOnly: ConstexprTernary(
            SetFunctionPtr,
            (std::is_rvalue_reference_v<std::tuple_element_t<0, typename SetDecomposer::ArgsTuple>>),
            false
        ),
        type: Factory<typename GetDecomposer::ReturnType>::Resolve(),
        getFunc: [](const void *instance) -> Var {
            if constexpr (GetDecomposer::IsFunctor)
                return Var::Assign(GetFunctionPtr());
            else if constexpr (GetDecomposer::IsMember)
                return Var::Assign(((const_cast<Type *>(reinterpret_cast<const Type *>(instance)))->*GetFunctionPtr)());
            else
                return Var::Assign((*GetFunctionPtr)());
        },
        setFunc: ConstexprTernary(
            SetFunctionPtr,
            ([]([[maybe_unused]] const void *instance, Var &value) -> Var {
                return Internal::Invoke<Type, SetFunctionPtr, true, SetDecomposer>(instance, &value, SetDecomposer::IndexSequence);
            }),
            nullptr
        ),
        signal: signal
    };
}

template<typename Type>
inline kF::Var kF::Meta::Data::set(const void *instance, Type &&value) const
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