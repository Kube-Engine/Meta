/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Meta custom constructor
 */

template<typename Type, typename ...Args> requires std::constructible_from<Type, Args...>
inline kF::Meta::Constructor::Descriptor kF::Meta::Constructor::Descriptor::Construct(void) noexcept
{
    static_assert(sizeof...(Args) > 0, "A meta custom constructor must have at least one argument");

    constexpr auto Dispatch = [](void *instance, Args &&...args) {
        new (instance) Type { std::forward<Args>(args)... };
    };

    using Decomposer = Internal::FunctionDecomposerHelper<void(Type::*)(Args...)>;

    return Descriptor {
        argsCount: sizeof...(Args),
        argTypeFunc: &Decomposer::ArgType,
        type: Factory<Type>::Resolve(),
        invokeFunc: [](void *instance, Var *args) -> bool {
            return Internal::Invoke<Type, Dispatch, Decomposer>(instance, args, Decomposer::IndexSequence).operator bool();
        }
    };
}

template<typename ...Args>
inline kF::Var kF::Meta::Constructor::invoke(Args &&...args) const
{
    Var var;
    auto returnType = type();

    if (returnType.isSmallOptimized()) {
        var.reserve<Var::UseSmallOptimization::Yes, Var::ShouldDestructInstance::No>(returnType);
        if (!invoke(var.data<Var::UseSmallOptimization::Yes>(), std::forward<Args>(args)...))
            return Var();
    } else {
        var.reserve<Var::UseSmallOptimization::No, Var::ShouldDestructInstance::No>(returnType);
        if (!invoke(var.data<Var::UseSmallOptimization::No>(), std::forward<Args>(args)...))
            return Var();
    }
    return var;
}

template<typename ...Args>
inline bool kF::Meta::Constructor::invoke(void *instance, Args &&...args) const
{
    kFAssert(sizeof...(Args) == argsCount(),
        throw std::logic_error("Meta::Constructor::invoke: Invalid number of arguments"));
    Var arguments[] { Var::Assign(std::forward<Args>(args))... };
    return (*_desc->invokeFunc)(instance, arguments);
}