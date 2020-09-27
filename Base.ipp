/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Meta Base
 */

template<typename Return, typename ...Args>
kF::Meta::Type kF::Meta::Internal::FunctionDecomposer<Return(Args...)>::ArgType(const std::size_t index) noexcept
{
    constexpr std::array<Type(*)(void), sizeof...(Args)> array { { &Factory<Args>::Resolve... } };

    return (*array[index])();
}

template<typename Type>
void kF::Meta::Internal::MakeDefaultConstructor(void *instance) noexcept_constructible(Type)
{
    new (instance) Type {};
}

template<typename Type>
void kF::Meta::Internal::MakeCopyConstructor(void *instance, const void *data) noexcept_copy_constructible(Type)
{
    new (instance) Type { *reinterpret_cast<const Type *>(data) };
}

template<typename Type>
void kF::Meta::Internal::MakeMoveConstructor(void *instance, void *data) noexcept_move_constructible(Type)
{
    new (instance) Type { std::move(*reinterpret_cast<Type *>(data)) };
}

template<typename Type>
void kF::Meta::Internal::MakeCopyAssignment(void *instance, const void *data) noexcept_copy_assignable(Type)
{
    *reinterpret_cast<Type *>(instance) = *reinterpret_cast<const Type *>(data);
}

template<typename Type>
void kF::Meta::Internal::MakeMoveAssignment(void *instance, void *data) noexcept_move_assignable(Type)
{
    *reinterpret_cast<Type *>(instance) = std::move(*reinterpret_cast<Type *>(data));
}

template<typename Type>
void kF::Meta::Internal::MakeDestructor(void *data) noexcept_destructible(Type)
{
    reinterpret_cast<Type *>(data)->~Type();
}

template<typename From, typename To>
kF::Var kF::Meta::Internal::MakeConverter(const void *from) noexcept_expr(static_cast<To>(std::declval<From>()))
{
    Var to;

    to.emplace<To>(static_cast<To>(*reinterpret_cast<const From *>(from)));
    return to;
}

template<typename From, typename To, auto FunctionPtr>
kF::Var kF::Meta::Internal::MakeCustomConverter(const void *from) noexcept_expr(FunctionPtr(std::declval<const From &>()))
{
    Var to;

    to.emplace<To>(std::invoke(FunctionPtr, *reinterpret_cast<const From *>(from)));
    return to;
}

template<typename Type, auto OperatorFunc, kF::Meta::UnaryOperator Operator>
kF::Var kF::Meta::Internal::MakeUnaryOperator(const void *data)
{
    return Var::Emplace<Type>((*OperatorFunc)(*reinterpret_cast<const Type *>(data)));
}

template<typename Type, auto OperatorFunc, kF::Meta::BinaryOperator Operator>
kF::Var kF::Meta::Internal::MakeBinaryOperator(const void *data, const Var &var)
{
    // Pointer only accepts std::size_t
    if constexpr (std::is_pointer_v<Type>)
        return Var::Emplace<Type>((*OperatorFunc)(*reinterpret_cast<const Type *>(data), var.directConvert<std::size_t>()));
    // If both operands are compatible, forward them
    else if (var.isCastAble<Type>())
        return Var::Emplace<Type>((*OperatorFunc)(*reinterpret_cast<const Type *>(data), var.as<Type>()));
    // If the type is either a float or NOT an integral, we can try to convert the opaque variable to Type without further check
    else if constexpr (std::is_floating_point_v<Type> || !std::is_integral_v<Type>)
        return Var::Emplace<Type>((*OperatorFunc)(*reinterpret_cast<const Type *>(data), var.directConvert<Type>()));
    // If the type is floating, we must convert opaque operand to either float or double builtin type
    else if (var.type().isFloating()) {
        Var tmp;
        if (var.type().isDouble())
            tmp.emplace<double, false>(static_cast<double>(*reinterpret_cast<const Type *>(data)));
        else
            tmp.emplace<float, false>(static_cast<float>(*reinterpret_cast<const Type *>(data)));
        return tmp.type().invokeOperator<Operator>(tmp.data(), var);
    // Else, tries to convert the opaque variable to Type
    } else
        return Var::Emplace<Type>((*OperatorFunc)(*reinterpret_cast<const Type *>(data), var.directConvert<Type>()));
}

template<typename Type, auto OperatorFunc, kF::Meta::AssignmentOperator Operator>
void kF::Meta::Internal::MakeAssignmentOperator(void *data, const Var &var)
{
    // Pointer only accepts std::size_t
    if constexpr (std::is_pointer_v<Type>)
        return (*OperatorFunc)(*reinterpret_cast<Type *>(data), var.directConvert<std::size_t>());
    // If both operands are compatible, forward them
    else if (var.isCastAble<Type>())
        return (*OperatorFunc)(*reinterpret_cast<Type *>(data), var.as<Type>());
    // If the type is either a float or NOT an integral, we can try to convert the opaque variable to Type without further check
    else if constexpr (std::is_floating_point_v<Type> || !std::is_integral_v<Type>)
        return (*OperatorFunc)(*reinterpret_cast<Type *>(data), var.directConvert<Type>());
    // If the type is floating, we must convert opaque operand to either float or double builtin type
    else if (var.type().isFloating()) { //
        Var tmp;
        if (var.type().isDouble()) {
            tmp.emplace<double, false>(static_cast<double>(*reinterpret_cast<const Type *>(data)));
            tmp.type().invokeOperator<Operator>(tmp.data(), var);
            auto res = static_cast<Type>(tmp.as<double>());
            return Factory<Type>::Resolve().copyAssign(data, &res);
        } else {
            tmp.emplace<float, false>(static_cast<float>(*reinterpret_cast<const Type *>(data)));
            tmp.type().invokeOperator<Operator>(tmp.data(), var);
            auto res = static_cast<Type>(tmp.as<float>());
            return Factory<Type>::Resolve().copyAssign(data, &res);
        }
    // Else, tries to convert the opaque variable to Type
    } else
        return (*OperatorFunc)(*reinterpret_cast<Type *>(data), var.directConvert<Type>());
}

template<typename Type, auto FunctionPtr, typename Decomposer, std::size_t ...Indexes>
kF::Var kF::Meta::Internal::Invoke(const void *instance, Var *args, const std::index_sequence<Indexes...> &sequence)
{
    using FunctionPtrType = decltype(FunctionPtr);

    constexpr auto Direct = [](Var *any, auto holder) {
        using ArgType = typename decltype(holder)::Type;
        using FlatArgType = std::remove_cvref_t<ArgType>;

        if constexpr (std::is_rvalue_reference_v<ArgType>) {
            if constexpr (std::is_same_v<FlatArgType, Var>)
                return std::move(*any);
            else
                return std::move(any->cast<FlatArgType>());
        } else if constexpr (std::is_lvalue_reference_v<ArgType>) {
            if constexpr (std::is_same_v<FlatArgType, Var>)
                return Var::Assign(*any);
            else if constexpr (!std::is_const_v<std::remove_reference_t<ArgType>>)
                return any->cast<FlatArgType>();
            else if (any->isCastAble<FlatArgType>())
                return any->as<FlatArgType>();
            else
                return any->directConvert<FlatArgType>();
        } else if constexpr (std::is_same_v<FlatArgType, Var>)
            return std::move(*any);
        else if (any->isCastAble<ArgType>())
            return any->storageType() == Var::StorageType::Value ? std::move(any->as<FlatArgType>()) : any->as<FlatArgType>();
        else
            return any->directConvert<FlatArgType>();
    };

    constexpr auto Dispatch = [](auto &&args) {
        if constexpr (std::is_same_v<typename Decomposer::ReturnType, void>) {
            std::apply(FunctionPtr, std::forward<decltype(args)>(args));
            return Var::Emplace<void>();
        } else
            return Var::Assign(std::apply(FunctionPtr, std::forward<decltype(args)>(args)));
    };

    if constexpr (Decomposer::IsFunctor || !Decomposer::IsMember)
        return Dispatch(
            std::forward_as_tuple(
                Direct(args + Indexes, TypeHolder<std::tuple_element_t<Indexes, typename Decomposer::ArgsTuple>>())...
            )
        );
    else
        return Dispatch(
            std::forward_as_tuple(
                const_cast<Type *>(reinterpret_cast<const Type *>(instance)),
                Direct(args + Indexes, TypeHolder<std::tuple_element_t<Indexes, typename Decomposer::ArgsTuple>>())...
            )
        );
}

template<typename Type, typename Decomposer, typename Functor, std::size_t ...Indexes>
kF::Var kF::Meta::Internal::Invoke(Functor &functor, [[maybe_unused]] const void *instance, Var *args, const std::index_sequence<Indexes...> &sequence)
{
    constexpr auto Direct = [](Var *any, auto holder) {
        using ArgType = typename decltype(holder)::Type;
        using FlatArgType = std::remove_cvref_t<ArgType>;

        if constexpr (std::is_rvalue_reference_v<ArgType>) {
            if constexpr (std::is_same_v<FlatArgType, Var>)
                return std::move(*any);
            else
                return std::move(any->cast<FlatArgType>());
        } else if constexpr (std::is_lvalue_reference_v<ArgType>) {
            if constexpr (std::is_same_v<FlatArgType, Var>)
                return Var::Assign(*any);
            else if constexpr (!std::is_const_v<std::remove_reference_t<ArgType>>)
                return any->cast<FlatArgType>();
            else if (any->isCastAble<FlatArgType>())
                return any->as<FlatArgType>();
            else
                return any->directConvert<FlatArgType>();
        } else if constexpr (std::is_same_v<FlatArgType, Var>)
            return std::move(*any);
        else if (any->isCastAble<ArgType>())
            return any->storageType() == Var::StorageType::Value ? std::move(any->as<FlatArgType>()) : any->as<FlatArgType>();
        else
            return any->directConvert<FlatArgType>();
    };

    constexpr auto Dispatch = [](auto &&functor, auto &&args) {
        if constexpr (std::is_same_v<typename Decomposer::ReturnType, void>) {
            std::apply(functor, std::forward<decltype(args)>(args));
            return Var::Emplace<void>();
        } else
            return Var::Assign(std::apply(functor, std::forward<decltype(args)>(args)));
    };

    if constexpr (Decomposer::IsFunctor || !Decomposer::IsMember)
        return Dispatch(
            std::forward<Functor>(functor),
            std::forward_as_tuple(
                Direct(args + Indexes, TypeHolder<std::tuple_element_t<Indexes, typename Decomposer::ArgsTuple>>())...
            )
        );
    else
        return Dispatch(
            std::forward<Functor>(functor),
            std::forward_as_tuple(
                const_cast<Type *>(reinterpret_cast<const Type *>(instance)),
                Direct(args + Indexes, TypeHolder<std::tuple_element_t<Indexes, typename Decomposer::ArgsTuple>>())...
            )
        );
}
