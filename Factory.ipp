/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Meta Factory
 */

template<typename RegisteredType>
inline void kF::Meta::FactoryBase<RegisteredType>::Register(const HashedName name, const HashedName specialization, const std::string_view &literal) noexcept_ndebug
{
    kFAssert(_Descriptor.name == 0,
        throw std::logic_error("Factory::Register: Type already registered"));
    _Descriptor.name = specialization;
    if (!literal.empty())
        _Descriptor.literal = literal;
    if (name == specialization)
        Resolver::RegisterMetaType(Resolve());
    else
        Resolver::RegisterMetaTemplateSpecialization(name, Resolve());
}

template<typename RegisteredType>
template<typename Base>
inline void kF::Meta::FactoryBase<RegisteredType>::RegisterBase(void) noexcept_ndebug
{
    kFAssert(!Resolve().findBase(FactoryBase<Base>::Resolve()),
        throw std::logic_error("Factory::RegisterBase: Base already registered"));
    _Descriptor.bases.push(FactoryBase<Base>::Resolve());
}

template<typename RegisteredType>
template<typename ...Args>
inline kF::Meta::Constructor kF::Meta::FactoryBase<RegisteredType>::RegisterConstructor(void) noexcept_ndebug
{
    using FunctionIdentifier = Internal::FunctionIdentifier<FactoryBase<RegisteredType>::RegisterConstructor<Args...>>;
    using DescriptorInstance = Internal::DescriptorInstance<Constructor::Descriptor, FunctionIdentifier>;

    auto &descriptor = DescriptorInstance::Initialize(Constructor::Descriptor::Construct<RegisteredType, Args...>());

    kFAssert(!Resolve().findConstructor<Args...>(),
        throw std::logic_error("Factory::RegisterConstructor: Constructor already registered"));
    _Descriptor.constructors.push(&descriptor);
    return Constructor(&descriptor);
}

template<typename RegisteredType>
template<typename To, auto FunctionPtr>
inline kF::Meta::Converter kF::Meta::FactoryBase<RegisteredType>::RegisterConverter(void) noexcept_ndebug
{
    using FunctionIdentifier = Internal::FunctionIdentifier<FactoryBase<RegisteredType>::RegisterConverter<To, FunctionPtr>>;
    using DescriptorInstance = Internal::DescriptorInstance<Converter::Descriptor, FunctionIdentifier>;

    auto &descriptor = DescriptorInstance::Initialize(Converter::Descriptor::Construct<RegisteredType, To, FunctionPtr>());

    kFAssert(!Resolve().findConverter(FactoryBase<To>::Resolve()),
        throw std::logic_error("Factory::RegisterConverter: Converter already registered"));
    _Descriptor.converters.push(&descriptor);
    return Converter(&descriptor);
}

template<typename RegisteredType>
template<auto FunctionPtr>
inline kF::Meta::Function kF::Meta::FactoryBase<RegisteredType>::RegisterFunction(const HashedName name) noexcept_ndebug
{
    using FunctionIdentifier = Internal::FunctionIdentifier<FactoryBase<RegisteredType>::RegisterFunction<FunctionPtr>>;
    using DescriptorInstance = Internal::DescriptorInstance<Function::Descriptor, FunctionIdentifier>;

    auto &descriptor = DescriptorInstance::Initialize(Function::Descriptor::Construct<RegisteredType, FunctionPtr>(name));

    kFAssert(!Resolve().findFunction(name),
        throw std::logic_error("Factory::RegisterFunction: Function already registered"));
    _Descriptor.functions.push(&descriptor);
    return Function(&descriptor);
}

template<typename RegisteredType>
template<auto GetFunctionPtr, auto SetFunctionPtr>
inline kF::Meta::Data kF::Meta::FactoryBase<RegisteredType>::RegisterData(const HashedName name) noexcept_ndebug
{
    using SetDecomposer = Internal::FunctionDecomposerHelper<decltype(SetFunctionPtr)>;

    if constexpr (SetFunctionPtr) {
        static_assert(std::tuple_size_v<typename SetDecomposer::ArgsTuple> == 1, "Meta-data's copy setter must take only one argument");
    }

    constexpr auto SetCopyFunctionPtr = [] {
        if constexpr (SetFunctionPtr) {
            if constexpr (std::is_rvalue_reference_v<std::tuple_element_t<0u, typename SetDecomposer::ArgsTuple>>)
                return static_cast<decltype(SetFunctionPtr)>(nullptr);
        }
        return SetFunctionPtr;
    }();
    constexpr auto SetMoveFunctionPtr = [] {
        if constexpr (SetFunctionPtr) {
            if constexpr (!std::is_rvalue_reference_v<std::tuple_element_t<0u, typename SetDecomposer::ArgsTuple>>)
                return static_cast<decltype(SetFunctionPtr)>(nullptr);
        }
        return SetFunctionPtr;
    }();

    return RegisterData<GetFunctionPtr, SetCopyFunctionPtr, SetMoveFunctionPtr>(name, signalName);
}

template<typename RegisteredType>
template<auto GetFunctionPtr, auto SetCopyFunctionPtr, auto SetMoveFunctionPtr>
inline kF::Meta::Data kF::Meta::FactoryBase<RegisteredType>::RegisterData(const HashedName name) noexcept_ndebug
{
    using FunctionIdentifier = Internal::FunctionIdentifier<FactoryBase<RegisteredType>::RegisterData<GetFunctionPtr, SetCopyFunctionPtr, SetMoveFunctionPtr>>;
    using DescriptorInstance = Internal::DescriptorInstance<Data::Descriptor, FunctionIdentifier>;

    auto &descriptor = DescriptorInstance::Initialize(
        Data::Descriptor::Construct<
            RegisteredType,
            GetFunctionPtr,
            SetCopyFunctionPtr,
            SetMoveFunctionPtr
        >(name)
    );

    kFAssert(!Resolve().findData(name),
        throw std::logic_error("Factory::RegisterData: Data already registered"));
    _Descriptor.datas.push(&descriptor);
    return Data(&descriptor);
}

template<typename RegisteredType>
template<auto SignalPtr>
inline kF::Meta::Signal kF::Meta::FactoryBase<RegisteredType>::RegisterSignal(const HashedName name) noexcept_ndebug
{
    using FunctionIdentifier = Internal::FunctionIdentifier<FactoryBase<RegisteredType>::RegisterSignal<SignalPtr>>;
    using DescriptorInstance = Internal::DescriptorInstance<Signal::Descriptor, FunctionIdentifier>;

    auto &descriptor = DescriptorInstance::Initialize(Signal::Descriptor::Construct<SignalPtr>(name));

    kFAssert(!Resolve().findSignal<SignalPtr>(),
        throw std::logic_error("Factory::RegisterSignal: Signal already registered"));
    _Descriptor.signals.push(&descriptor);
    return Signal(&descriptor);
}