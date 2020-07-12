/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Meta custom constructor
 */

template<typename Type, typename ...Args>
kF::Meta::Constructor::Descriptor kF::Meta::Constructor::Descriptor::Construct(void) noexcept
{
    static_assert(sizeof...(Args) > 0);

    constexpr auto dispatch = [](void *instance, Args &&...args) {
        new (instance) Type { std::forward<Args>(args)... };
    };

    using Decomposer = Internal::FunctionDecomposerHelper<void(Type::*)(Args...)>;

    return Descriptor {
        argsCount: sizeof...(Args),
        argTypeFunc: &Decomposer::ArgType,
        type: Factory<Type>::Resolve(),
        invokeFunc: [](void *instance, Var *args) -> bool {
            return Internal::Invoke<Type, dispatch, Decomposer>(instance, args, Decomposer::IndexSequence).operator bool();
        }
    };
}

template<typename ...Args>
kF::Var kF::Meta::Constructor::invoke(Args &&...args) const
{
    Var var;
    auto returnType = type();

    if (returnType.isTrivial()) {
        var.reserve<true>(returnType);
        if (!invoke(var.data<true>(), std::forward<Args>(args)...))
            return Var();
    } else {
        var.reserve<false>(returnType);
        if (!invoke(var.data<false>(), std::forward<Args>(args)...))
            return Var();
    }
    return var;
}

template<typename ...Args>
bool kF::Meta::Constructor::invoke(void *instance, Args &&...args) const
{
    kFAssert(sizeof...(Args) == argsCount(),
        throw std::logic_error("Meta::Constructor::invoke: Invalid number of arguments"));
    Var arguments[] { Var::Assign(std::forward<Args>(args))... };
    return (*_desc->invokeFunc)(instance, arguments);
}