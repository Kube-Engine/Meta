/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Meta signal
 */

template<auto SignalPtr>
kF::Meta::Signal::Descriptor kF::Meta::Signal::Descriptor::Construct(const HashedName name, std::vector<HashedName> &&arguments) noexcept_ndebug
{
    using Decomposer = Internal::FunctionDecomposerHelper<decltype(SignalPtr)>;

    kFAssert(std::tuple_size_v<typename Decomposer::ArgsTuple> == arguments.size(),
        throw std::runtime_error("Meta::Signal: Invalid number of arguments for given signal signature"));
    return Descriptor {
        signalPtr: Internal::GetFunctionIdentifier<SignalPtr>(),
        name: name,
        arguments: std::move(arguments)
    };
}

template<typename Receiver, typename Functor, typename Decomposer>
kF::Meta::OpaqueFunctor kF::Meta::OpaqueFunctor::Construct(const Receiver *receiver, Functor &&functor) noexcept_ndebug
{
    return OpaqueFunctor {
        receiver: reinterpret_cast<const void *>(receiver),
        invokeFunc: [](Var &data, const void *receiver, Var *args) -> bool {
            using FunctorType = std::remove_cvref_t<Functor>;

            return Internal::Invoke<Receiver, Decomposer, Functor>(data.as<FunctorType>(), receiver, args, Decomposer::IndexSequence).operator bool();
        },
        data: std::forward<Functor>(functor)
    };
}

template<typename Sender, typename Receiver, typename Functor>
kF::Meta::Connection kF::Meta::Signal::connect(const Sender *sender, const Receiver *receiver, Functor &&functor) noexcept_ndebug
{
    using Decomposer = Internal::FunctionDecomposerHelper<std::remove_cvref_t<Functor>>;

    const OpaqueFunctor *opaqueFunctor;

    kFAssert(argsCount() == std::tuple_size_v<typename Decomposer::ArgsTuple>,
        throw std::runtime_error("Meta::Signal::connect: Invalid number of arguments of slot"));
    if (!_desc->freeSlots.empty()) {
        auto &slot = _desc->slots[_desc->freeSlots.back()];
        _desc->freeSlots.pop_back();
        opaqueFunctor = slot.opaqueFunctor.get();
        slot.sender = reinterpret_cast<const void *>(sender);
        *slot.opaqueFunctor = OpaqueFunctor::Construct<Receiver, Functor, Decomposer>(receiver, std::forward<Functor>(functor));
    } else {
        opaqueFunctor = _desc->slots.emplace_back(Slot {
            sender: reinterpret_cast<const void *>(sender),
            opaqueFunctor: std::make_unique<OpaqueFunctor>(OpaqueFunctor::Construct<Receiver, Functor, Decomposer>(receiver, std::forward<Functor>(functor)))
        }).opaqueFunctor.get();
    }
    return Connection(*this, reinterpret_cast<const void *>(sender), opaqueFunctor);
}

template<typename Sender, typename Functor>
kF::Meta::Connection kF::Meta::Signal::connect(const Sender *sender, Functor &&functor) noexcept_ndebug
{
    return connect(sender, static_cast<const void *>(nullptr), std::forward<Functor>(functor));
}

template<typename Sender, typename ...Args>
void kF::Meta::Signal::emit(const Sender *sender, Args &&...args)
{
    kFAssert(sizeof...(Args) == argsCount(),
        throw std::runtime_error("Meta::Signal::emit: Invalid parameters, given " + std::to_string(sizeof...(Args)) + " but expected " + std::to_string(argsCount()) + " argument(s)"));
    Var arguments[] { Var::Assign(std::forward<Args>(args))... };
    for (const auto &slot : _desc->slots) {
        if (reinterpret_cast<const void *>(sender) != slot.sender || !slot.opaqueFunctor->invokeFunc)
            continue;
        else if (!slot.opaqueFunctor->invokeFunc(slot.opaqueFunctor->data, slot.opaqueFunctor->receiver, arguments))
            throw std::runtime_error("Meta::Signal::emit: Invalid slot signature");
    }
}

void kF::Meta::Signal::disconnect(const OpaqueFunctor *opaqueFunctor) noexcept
{
    for (auto i = 0u; auto &slot : _desc->slots) {
        if (slot.opaqueFunctor.get() != opaqueFunctor) {
            ++i;
            continue;
        }
        slot.sender = nullptr;
        slot.opaqueFunctor->invokeFunc = nullptr;
        _desc->freeSlots.emplace_back(i);
        break;
    }
}