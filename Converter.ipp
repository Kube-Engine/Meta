/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Meta Converter
 */

template<typename From, typename To, auto FunctionPtr>
inline kF::Meta::Converter::Descriptor kF::Meta::Converter::Descriptor::Construct(void) noexcept
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

    if constexpr (IsCustom) {
        static_assert(std::is_invocable_v<decltype(FunctionPtr), const From &>, "A meta converter may only take custom functions that take the type to convert type as const reference");
        static_assert(std::is_same_v<decltype(std::invoke(FunctionPtr, std::declval<const From &>())), To>, "A meta converter may only take custom functions that return the a value to converted type");
    } else {
        static_assert(std::is_convertible_v<From, To>, "A meta converter without explicit function must have converted target being static convertible");
    }

    return Descriptor {
        .convertType = Factory<To>::Resolve(),
        .convertFunc = [](const void *from, void *to) {
            if constexpr (IsCustom)
                new (to) To { std::invoke(FunctionPtr, *reinterpret_cast<const From *>(from)) };
            else
                new (to) To { static_cast<To>(*reinterpret_cast<const From *>(from)) };
        }
    };
}

template<kF::Var::ShouldDestructInstance DestructInstance>
void kF::Meta::Converter::invoke(const Var &from, Var &to) const
{
    if (auto type = convertType(); type.isSmallOptimized()) {
        to.reserve<Var::UseSmallOptimization::Yes, DestructInstance>(type);
        invoke(from.data(), to.data<Var::UseSmallOptimization::Yes>());
    } else {
        to.reserve<Var::UseSmallOptimization::No, DestructInstance>(type);
        invoke(from.data(), to.data<Var::UseSmallOptimization::No>());
    }
}