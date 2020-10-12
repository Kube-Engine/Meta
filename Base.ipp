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
inline void kF::Meta::Internal::MakeDefaultConstructor(void *instance) noexcept_constructible(Type)
{
    new (instance) Type {};
}

template<typename Type>
inline void kF::Meta::Internal::MakeCopyConstructor(void *instance, const void *data) noexcept_copy_constructible(Type)
{
    new (instance) Type { *reinterpret_cast<const Type *>(data) };
}

template<typename Type>
inline void kF::Meta::Internal::MakeMoveConstructor(void *instance, void *data) noexcept_move_constructible(Type)
{
    new (instance) Type { std::move(*reinterpret_cast<Type *>(data)) };
}

template<typename Type>
inline void kF::Meta::Internal::MakeCopyAssignment(void *instance, const void *data) noexcept_copy_assignable(Type)
{
    *reinterpret_cast<Type *>(instance) = *reinterpret_cast<const Type *>(data);
}

template<typename Type>
inline void kF::Meta::Internal::MakeMoveAssignment(void *instance, void *data) noexcept_move_assignable(Type)
{
    *reinterpret_cast<Type *>(instance) = std::move(*reinterpret_cast<Type *>(data));
}

template<typename Type>
inline void kF::Meta::Internal::MakeDestructor(void *data) noexcept_destructible(Type)
{
    reinterpret_cast<Type *>(data)->~Type();
}

template<typename Type, auto OperatorFunc, kF::Meta::UnaryOperator Operator>
inline kF::Var kF::Meta::Internal::MakeUnaryOperator(const void *data)
{
    return Var::Emplace<Type>((*OperatorFunc)(*reinterpret_cast<const Type *>(data)));
}

template<typename Type, auto OperatorFunc, kF::Meta::BinaryOperator Operator>
inline kF::Var kF::Meta::Internal::MakeBinaryOperator(const void *data, const Var &var)
{
    // Pointer only accepts std::size_t
    if constexpr (std::is_pointer_v<Type>) {
        if (var.isCastAble<std::size_t>()) [[likely]]
            return Var::Emplace<Type>((*OperatorFunc)(*reinterpret_cast<const Type *>(data), var.as<std::size_t>()));
        else [[unlikely]]
            return Var::Emplace<Type>((*OperatorFunc)(*reinterpret_cast<const Type *>(data), var.convertExplicit<std::size_t>()));
    // If both operands are compatible, forward them
    } else if (var.isCastAble<Type>())
        return Var::Emplace<Type>((*OperatorFunc)(*reinterpret_cast<const Type *>(data), var.as<Type>()));
    // If the type is either a float or NOT an integral, we can try to convert the opaque variable to Type without further check
    else if constexpr (std::is_floating_point_v<Type> || !std::is_integral_v<Type>) {
        return Var::Emplace<Type>((*OperatorFunc)(*reinterpret_cast<const Type *>(data), var.convertExplicit<Type>()));
    // If the type is floating, we must convert opaque operand to either float or double builtin type
    } else if (var.type().isFloating()) {
        constexpr auto compute = [](const auto &lhs, const auto &rhs) noexcept {
            switch (Operator) {
            case BinaryOperator::Addition:
                return Internal::BinaryAddition(lhs, rhs);
            case BinaryOperator::Substraction:
                return Internal::BinarySubstraction(lhs, rhs);
            case BinaryOperator::Multiplication:
                return Internal::BinaryMultiplication(lhs, rhs);
            case BinaryOperator::Division:
                return Internal::BinaryDivision(lhs, rhs);
            case BinaryOperator::Modulo:
                return Internal::BinaryModulo(lhs, rhs);
            default:
                return decltype(lhs){};
            }
        };
        if (var.type().isDouble())
            return Var::Emplace<double>(compute(static_cast<double>(*reinterpret_cast<const Type *>(data)), var.as<double>()));
        else
            return Var::Emplace<float>(compute(static_cast<float>(*reinterpret_cast<const Type *>(data)), var.as<float>()));
    // Else, tries to convert the opaque variable to Type
    } else
        return Var::Emplace<Type>((*OperatorFunc)(*reinterpret_cast<const Type *>(data), var.convertExplicit<Type>()));
}

template<typename Type, auto OperatorFunc, kF::Meta::AssignmentOperator Operator>
inline void kF::Meta::Internal::MakeAssignmentOperator(void *data, const Var &var)
{
    // Pointer only accepts std::size_t
    if constexpr (std::is_pointer_v<Type>)
        if (var.isCastAble<std::size_t>()) [[likely]]
            return (*OperatorFunc)(*reinterpret_cast<Type *>(data), var.as<std::size_t>());
        else [[unlikely]]
            return (*OperatorFunc)(*reinterpret_cast<Type *>(data), var.convertExplicit<std::size_t>());
    // If both operands are compatible, forward them
    else if (var.isCastAble<Type>())
        return (*OperatorFunc)(*reinterpret_cast<Type *>(data), var.as<Type>());
    // If the type is either a float or NOT an integral, we can try to convert the opaque variable to Type without further check
    else if constexpr (std::is_floating_point_v<Type> || !std::is_integral_v<Type>)
        return (*OperatorFunc)(*reinterpret_cast<Type *>(data), var.convertExplicit<Type>());
    // If the left operand is integral and right operand is floating, we must compute the expression differently
    else if (var.type().isFloating()) {
        constexpr auto compute = [](auto &lhs, const auto &rhs) noexcept {
            switch (Operator) {
            case AssignmentOperator::Addition:
                return Internal::AssignmentAddition(lhs, rhs);
            case AssignmentOperator::Substraction:
                return Internal::AssignmentSubstraction(lhs, rhs);
            case AssignmentOperator::Multiplication:
                return Internal::AssignmentMultiplication(lhs, rhs);
            case AssignmentOperator::Division:
                return Internal::AssignmentDivision(lhs, rhs);
            case AssignmentOperator::Modulo:
                return Internal::AssignmentModulo(lhs, rhs);
            default:
                return;
            }
        };
        if (var.type().isDouble()) {
            double tmp = static_cast<double>(*reinterpret_cast<const Type *>(data));
            compute(tmp, var.as<double>());
            *reinterpret_cast<Type *>(data) = static_cast<Type>(tmp);
        } else {
            float tmp = static_cast<float>(*reinterpret_cast<const Type *>(data));
            compute(tmp, var.as<float>());
            *reinterpret_cast<Type *>(data) = static_cast<Type>(tmp);
        }
    // Else, tries to convert the integeropaque variable to Type
    } else
        return (*OperatorFunc)(*reinterpret_cast<Type *>(data), var.convertExplicit<Type>());
}

template<typename ArgType, bool AllowImplicitMove>
inline decltype(auto) kF::Meta::Internal::ForwardArgument(Var *any)
{
    using FlatArgType = std::remove_cvref_t<ArgType>;

    if constexpr (std::is_same_v<FlatArgType, Var>) { // ArgType: (const) Var (&)(&)
        if constexpr (std::is_rvalue_reference_v<ArgType>) { // ArgType: Var &&
            return std::move(*any); // Move the instance
        } else if constexpr (std::is_lvalue_reference_v<ArgType>) { // ArgType: (const) Var &
            return ConstexprTernaryRef(std::is_const_v<std::remove_reference_t<ArgType>>, static_cast<const Var &>(*any), static_cast<Var &>(*any)); // Forward the reference
        } else { // ArgType: Var
            if constexpr (AllowImplicitMove) { // Implicit move allowed so temporary values can be moved instead of copied
                if (const auto storage = any->storageType(); storage == Var::StorageType::Value || storage == Var::StorageType::ValueOptimized)
                    return std::move(*any); // Move the value
            }
            return *any; // Deep copy either a value (maybe costly) or a reference (always cheap)
        }
    } else { // ArgType: typename Type
        if constexpr (std::is_rvalue_reference_v<ArgType>) { // ArgType: Type &&
            return std::move(any->cast<FlatArgType>()); // Move the instance
        } else if constexpr (std::is_lvalue_reference_v<ArgType>) { // ArgType: (const) Type &
            if constexpr (!std::is_const_v<std::remove_reference_t<ArgType>>) // ArgType: Type &
                return static_cast<FlatArgType &>(any->cast<FlatArgType>()); // Try to forward reference
            else { // ArgType: const Type & -> use converter if needed
                if constexpr (AllowImplicitMove) {
                    if (!any->isCastAble<FlatArgType>()) [[unlikely]] // Perfect match
                        any->emplace<FlatArgType>(any->convertExplicit<FlatArgType>()); // Convert any to FlatArgType
                    return static_cast<const FlatArgType &>(any->as<FlatArgType>()); // Forward the reference
                } else {
                    return static_cast<const FlatArgType &>(any->cast<FlatArgType>()); // Cast any to FlatArgType
                }
            }
        } else { // ArgType: Type
            if (any->isCastAble<FlatArgType>()) [[likely]] { // Perfect match
                if constexpr (AllowImplicitMove) { // Implicit move allowed so temporary values can be moved instead of copied
                    if (const auto storage = any->storageType(); storage == Var::StorageType::Value || storage == Var::StorageType::ValueOptimized)
                        return FlatArgType { std::move(any->as<FlatArgType>()) }; // Move the value
                }
                return FlatArgType { any->as<FlatArgType>() }; // Deep copy of the value (maybe costly)
            } else [[unlikely]]
                return FlatArgType { any->convertExplicit<FlatArgType>() }; // Call the converter
        }
    }
}

template<typename Type, auto FunctionPtr, bool AllowImplicitMove, typename Decomposer, std::size_t ...Indexes>
inline kF::Var kF::Meta::Internal::Invoke(const void *instance, Var *args, const std::index_sequence<Indexes...> &sequence)
{
    using FunctionPtrType = decltype(FunctionPtr);

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
                ForwardArgument<std::tuple_element_t<Indexes, typename Decomposer::ArgsTuple>, AllowImplicitMove>(args + Indexes)...
            )
        );
    else
        return Dispatch(
            std::forward_as_tuple(
                const_cast<Type *>(reinterpret_cast<const Type *>(instance)),
                ForwardArgument<std::tuple_element_t<Indexes, typename Decomposer::ArgsTuple>, AllowImplicitMove>(args + Indexes)...
            )
        );
}

template<typename Type, bool AllowImplicitMove, typename Decomposer, typename Functor, std::size_t ...Indexes>
inline kF::Var kF::Meta::Internal::Invoke(Functor &functor, [[maybe_unused]] const void *instance, Var *args, const std::index_sequence<Indexes...> &sequence)
{
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
                ForwardArgument<std::tuple_element_t<Indexes, typename Decomposer::ArgsTuple>, AllowImplicitMove>(args + Indexes)...
            )
        );
    else
        return Dispatch(
            std::forward<Functor>(functor),
            std::forward_as_tuple(
                const_cast<Type *>(reinterpret_cast<const Type *>(instance)),
                ForwardArgument<std::tuple_element_t<Indexes, typename Decomposer::ArgsTuple>, AllowImplicitMove>(args + Indexes)...
            )
        );
}
