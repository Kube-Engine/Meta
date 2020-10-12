/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Meta Function
 */

template<typename Type, auto FunctionPtr>
inline kF::Meta::Function::Descriptor kF::Meta::Function::Descriptor::Construct(const HashedName name) noexcept
{
    using FunctionType = decltype(FunctionPtr);
    using Decomposer = Meta::Internal::FunctionDecomposerHelper<FunctionType>;

    return Descriptor {
        name: name,
        argsCount: std::tuple_size_v<typename Decomposer::ArgsTuple>,
        isStatic: !std::is_member_function_pointer_v<FunctionType>,
        isConst: Decomposer::IsConst,
        returnType: Factory<typename Decomposer::ReturnType>::Resolve(),
        argTypeFunc: &Decomposer::ArgType,
        invokeFunc: [](const void *instance, Var *args) {
            return Internal::Invoke<Type, FunctionPtr, true, Decomposer>(instance, args, Decomposer::IndexSequence);
        }
    };
}

template<typename ...Args>
inline kF::Var kF::Meta::Function::invoke(const void *instance, Args &&...args) const
{
    kFAssert(sizeof...(Args) == argsCount(),
        return Var());
    Var arguments[] { Var::Assign(std::forward<Args>(args))... };
    return (*_desc->invokeFunc)(instance, arguments).operator bool();
}