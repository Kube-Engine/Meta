/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Meta signal
 */

template<auto SignalPtr>
inline kF::Meta::Signal::Descriptor kF::Meta::Signal::Descriptor::Construct(const HashedName name) noexcept
{
    using Decomposer = Internal::FunctionDecomposerHelper<decltype(SignalPtr)>;

    return Descriptor {
        signalPtr: Internal::GetFunctionIdentifier<SignalPtr>(),
        name: name,
        argsCount: Decomposer::IndexSequence.size()
    };
}
