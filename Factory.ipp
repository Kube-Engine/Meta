/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Meta Factory
 */

template<typename RegisteredType>
inline void kF::Meta::FactoryBase<RegisteredType>::Register(const HashedName name) noexcept_ndebug
{
    kFAssert(_Descriptor.name == 0,
        throw std::logic_error("Factory::Register: Type already registered"));
    _Descriptor.name = name;
    Resolver::RegisterMetaType(Resolve());
}

template<typename RegisteredType>
template<typename Base>
inline void kF::Meta::FactoryBase<RegisteredType>::RegisterBase(void) noexcept_ndebug
{
    kFAssert(!GetType().findBase(FactoryBase<Base>::Resolve()),
        throw std::logic_error("Factory::RegisterBase: Base already registered"));
    _Descriptor.bases.push(FactoryBase<Base>::Resolve());
}

template<typename RegisteredType>
template<typename ...Args>
inline void kF::Meta::FactoryBase<RegisteredType>::RegisterConstructor(void) noexcept_ndebug
{
    static auto Descriptor = Constructor::Descriptor::Construct<RegisteredType, Args...>();

    kFAssert(!GetType().findConstructor<Args...>(),
        throw std::logic_error("Factory::RegisterConstructor: Constructor already registered"));
    _Descriptor.constructors.push(&Descriptor);
}

template<typename RegisteredType>
template<typename To, auto FunctionPtr>
inline void kF::Meta::FactoryBase<RegisteredType>::RegisterConverter(void) noexcept_ndebug
{
    static auto Descriptor { Converter::Descriptor::Construct<RegisteredType, To, FunctionPtr>() };

    kFAssert(!GetType().findConverter(FactoryBase<To>::Resolve()),
        throw std::logic_error("Factory::RegisterConverter: Converter already registered"));
    _Descriptor.converters.push(&Descriptor);
}

template<typename RegisteredType>
template<auto FunctionPtr>
inline void kF::Meta::FactoryBase<RegisteredType>::RegisterFunction(const HashedName name) noexcept_ndebug
{
    static auto Descriptor = Function::Descriptor::Construct<RegisteredType, FunctionPtr>(name);

    kFAssert(!GetType().findFunction(name),
        throw std::logic_error("Factory::RegisterFunction: Function already registered"));
    _Descriptor.functions.push(&Descriptor);
}

template<typename RegisteredType>
template<auto GetFunctionPtr, auto SetFunctionPtr>
inline void kF::Meta::FactoryBase<RegisteredType>::RegisterData(const HashedName name, const Signal signal) noexcept_ndebug
{
    static auto Descriptor = Data::Descriptor::Construct<RegisteredType, GetFunctionPtr, SetFunctionPtr>(name, signal);

    kFAssert(!GetType().findData(name),
        throw std::logic_error("Factory::RegisterData: Data already registered"));
    _Descriptor.datas.push(&Descriptor);
}

template<typename RegisteredType>
template<auto SignalPtr>
inline void kF::Meta::FactoryBase<RegisteredType>::RegisterSignal(const HashedName name) noexcept_ndebug
{
    static auto Descriptor = Signal::Descriptor::Construct<SignalPtr>(name);

    kFAssert(!GetType().findSignal<SignalPtr>(),
        throw std::logic_error("Factory::RegisterSignal: Signal already registered"));
    _Descriptor.signals.push(&Descriptor);
}