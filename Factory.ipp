/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Meta Factory
 */

template<typename Type>
void kF::Meta::FactoryBase<Type>::Register(const HashedName name) noexcept_ndebug
{
    kFAssert(GetDescriptor().name == 0,
        throw std::logic_error("Factory::Register: Type already registered"));
    GetDescriptor().name = name;
    Resolver::RegisterMetaType(Resolve());
}

template<typename Type>
template<typename Base>
void kF::Meta::FactoryBase<Type>::RegisterBase(void) noexcept_ndebug
{
    kFAssert(!Meta::Type(&GetDescriptor()).findBase(FactoryBase<Base>::Resolve()),
        throw std::logic_error("Factory::RegisterBase: Base already registered"));
    GetDescriptor().bases.push(FactoryBase<Base>::Resolve());
}

template<typename Type>
template<typename ...Args>
void kF::Meta::FactoryBase<Type>::RegisterConstructor(void) noexcept_ndebug
{
    static auto descriptor { Constructor::Descriptor::Construct<Type, Args...>() };

    kFAssert(!Meta::Type(&GetDescriptor()).findConstructor<Args...>(),
        throw std::logic_error("Factory::RegisterConstructor: Constructor already registered"));
    GetDescriptor().constructors.push(&descriptor);
}

template<typename Type>
template<typename To, auto FunctionPtr>
void kF::Meta::FactoryBase<Type>::RegisterConverter(void) noexcept_ndebug
{
    static_assert(!std::is_same_v<Type, To>, "A meta converter must not convert a type to itself");

    constexpr bool IsCustom = [] {
        if constexpr (std::is_same_v<decltype(FunctionPtr), std::nullptr_t>)
            return false;
        else if constexpr (std::is_convertible_v<decltype(FunctionPtr), std::nullptr_t>)
            return FunctionPtr != nullptr;
        else
            return true;
    }();

    static auto descriptor {
        Converter::Descriptor::Construct<To,
            ConstexprTernary(!IsCustom,
                (&Internal::MakeConverter<Type, To>),
                (&Internal::MakeCustomConverter<Type, To, FunctionPtr>)
            )
        >()
    };

    kFAssert(!Meta::Type(&GetDescriptor()).findConverter(FactoryBase<To>::Resolve()),
        throw std::logic_error("Factory::RegisterConverter: Converter already registered"));
    GetDescriptor().converters.push(&descriptor);
}

template<typename Type>
template<auto FunctionPtr>
void kF::Meta::FactoryBase<Type>::RegisterFunction(const HashedName name) noexcept_ndebug
{
    static auto descriptor { Function::Descriptor::Construct<Type, FunctionPtr>(name) };

    kFAssert(!Meta::Type(&GetDescriptor()).findFunction(name),
        throw std::logic_error("Factory::RegisterFunction: Function already registered"));
    GetDescriptor().functions.push(&descriptor);
}

template<typename Type>
template<auto GetFunctionPtr, auto SetFunctionPtr>
void kF::Meta::FactoryBase<Type>::RegisterData(const HashedName name) noexcept_ndebug
{
    static auto descriptor { Data::Descriptor::Construct<Type, GetFunctionPtr, SetFunctionPtr>(name) };

    kFAssert(!Meta::Type(&GetDescriptor()).findData(name),
        throw std::logic_error("Factory::RegisterData: Data already registered"));
    GetDescriptor().datas.push(&descriptor);
}

template<typename Type>
template<auto SignalPtr>
void kF::Meta::FactoryBase<Type>::RegisterSignal(const HashedName name) noexcept_ndebug
{
    static auto descriptor { Signal::Descriptor::Construct<SignalPtr>(name) };

    kFAssert(!Meta::Type(&GetDescriptor()).findSignal<SignalPtr>(),
        throw std::logic_error("Factory::RegisterSignal: Signal already registered"));
    GetDescriptor().signals.push(&descriptor);
}