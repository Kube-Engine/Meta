/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Meta Converter
 */

template<typename Type, kF::Meta::Converter::ConvertSignature FunctionPtr>
kF::Meta::Converter::Descriptor kF::Meta::Converter::Descriptor::Construct(void) noexcept
{
    return Descriptor {
        .convertType = Factory<Type>::Resolve(),
        .convertFunc = FunctionPtr
    };
}
