/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Meta Data
 */

template<typename Type, auto GetFunctionPtr, auto SetCopyFunctionPtr, auto SetMoveFunctionPtr>
inline kF::Meta::Data::Descriptor kF::Meta::Data::Descriptor::Construct(const HashedName name, const Signal signal) noexcept
{
    using GetFunctionType = decltype(GetFunctionPtr);
    using GetDecomposer = Internal::FunctionDecomposerHelper<GetFunctionType>;
    using SetCopyFunctionType = decltype(SetCopyFunctionPtr);
    using SetCopyDecomposer = Internal::FunctionDecomposerHelper<SetCopyFunctionType>;
    using SetMoveFunctionType = decltype(SetMoveFunctionPtr);
    using SetMoveDecomposer = Internal::FunctionDecomposerHelper<SetMoveFunctionType>;

    static_assert(GetFunctionPtr != nullptr, "A meta data must at least have a getter");
    static_assert(std::tuple_size_v<typename GetDecomposer::ArgsTuple> == 0, "A meta data must have a getter that take no argument");

    using FlatGetterReturnType = std::remove_cvref_t<typename Internal::FunctionDecomposerHelper<GetFunctionType>::ReturnType>;

    if constexpr (SetCopyFunctionPtr) {
        static_assert(std::tuple_size_v<typename SetCopyDecomposer::ArgsTuple> == 1, "Meta-data's copy setter must take only one argument");
        using SetterArg = std::tuple_element_t<0, typename SetCopyDecomposer::ArgsTuple>;
        static_assert(std::is_same_v<SetterArg, const FlatGetterReturnType &>, "Meta-data's copy setter must take the data type as constant LValue reference in parameter");
    }

    if constexpr (SetMoveFunctionPtr) {
        static_assert(std::tuple_size_v<typename SetMoveDecomposer::ArgsTuple> == 1, "Meta-data's move setter must take only one argument");
        using SetterArg = std::tuple_element_t<0, typename SetMoveDecomposer::ArgsTuple>;
        static_assert(std::is_same_v<SetterArg, FlatGetterReturnType &&>, "Meta-data's move setter must take the data type as RValue in parameter");
    }

    return Descriptor {
        name: name,
        isStatic: GetDecomposer::IsFunctor || !GetDecomposer::IsMember,
        type: Factory<typename GetDecomposer::ReturnType>::Resolve(),
        getFunc: [](const void *instance) -> Var {
            if constexpr (GetDecomposer::IsFunctor)
                return Var::Assign(GetFunctionPtr());
            else if constexpr (GetDecomposer::IsMember)
                return Var::Assign(((const_cast<Type *>(reinterpret_cast<const Type *>(instance)))->*GetFunctionPtr)());
            else
                return Var::Assign((*GetFunctionPtr)());
        },
        setCopyFunc: ConstexprTernary(
            SetCopyFunctionPtr,
            ([]([[maybe_unused]] const void *instance, const Var &value) -> Var {
                return Internal::Invoke<Type, SetCopyFunctionPtr, true, SetCopyDecomposer>(instance, const_cast<Var *>(&value), SetCopyDecomposer::IndexSequence);
            }),
            nullptr
        ),
        setMoveFunc: ConstexprTernary(
            SetMoveFunctionPtr,
            ([]([[maybe_unused]] const void *instance, Var &&value) -> Var {
                return Internal::Invoke<Type, SetMoveFunctionPtr, true, SetMoveDecomposer>(instance, &value, SetMoveDecomposer::IndexSequence);
            }),
            nullptr
        ),
        signal: signal
    };
}

template<typename Type>
inline kF::Var kF::Meta::Data::set(const void *instance, Type &&value) const
{
    constexpr auto IsRValue = std::is_rvalue_reference_v<decltype(value)>;

    if constexpr (IsRValue) {
        kFAssert(isCopySettable() || isMoveSettable(),
            throw std::runtime_error("Meta::Data::set: Value is RValue and data is not move settable"));
    } else {
        kFAssert(isCopySettable(),
            throw std::runtime_error("Meta::Data::set: Value is constant LValue and data is not copy settable"));
    }

    if constexpr (std::is_same_v<std::remove_cvref_t<Type>, Var>) {
        if constexpr (IsRValue) {
            if (isMoveSettable()) [[likely]]
                return (*_desc->setMoveFunc)(instance, std::move(value));
        }
        return (*_desc->setCopyFunc)(instance, value);
    } else {
        if constexpr (IsRValue) {
            if (isMoveSettable()) [[likely]]
                return (*_desc->setMoveFunc)(instance, kF::Var::Assign(std::forward<Type>(value)));
        }
        return (*_desc->setCopyFunc)(instance, kF::Var::Assign(std::forward<Type>(value)));
    }
}