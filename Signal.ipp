/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Meta signal
 */

template<auto SignalPtr>
kF::Meta::Signal::Descriptor kF::Meta::Signal::Descriptor::Construct(const HashedName name) noexcept
{
    using Decomposer = Internal::FunctionDecomposerHelper<decltype(SignalPtr)>;

    return Descriptor {
        signalPtr: Internal::GetFunctionIdentifier<SignalPtr>(),
        name: name,
        argsCount: Decomposer::IndexSequence.size()
    };
}

template<typename Receiver, typename Functor, typename Decomposer>
kF::Meta::OpaqueFunctor kF::Meta::OpaqueFunctor::Construct(const void *receiver, Functor &&functor) noexcept
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
kF::Meta::Connection kF::Meta::Signal::connect(const Sender &sender, const Receiver &receiver, Functor &&functor) noexcept_ndebug
{
    using Decomposer = Internal::FunctionDecomposerHelper<std::remove_cvref_t<Functor>>;

    constexpr auto getPtr = [](const auto &input) -> const void * {
        if constexpr (std::is_same_v<std::remove_cvref_t<decltype(input)>, std::nullptr_t>)
            return static_cast<const void *>(input);
        else if constexpr (std::is_pointer_v<std::remove_cvref_t<decltype(input)>>)
            return static_cast<const void *>(input);
        else
            return static_cast<const void *>(&input);
    };

    const void * const senderPtr = getPtr(sender);
    const void * const receiverPtr = getPtr(receiver);
    const OpaqueFunctor *opaqueFunctor;

    kFAssert(argsCount() == std::tuple_size_v<typename Decomposer::ArgsTuple>,
        throw std::runtime_error("Meta::Signal::connect: Invalid number of arguments of slot"));
    if (!_desc->freeSlots.empty()) {
        auto &slot = _desc->slots[_desc->freeSlots.back()];
        _desc->freeSlots.pop_back();
        opaqueFunctor = slot.opaqueFunctor.get();
        slot.sender = senderPtr;
        *slot.opaqueFunctor = OpaqueFunctor::Construct<Receiver, Functor, Decomposer>(receiverPtr, std::forward<Functor>(functor));
    } else {
        opaqueFunctor = _desc->slots.emplace_back(Slot {
            sender: senderPtr,
            opaqueFunctor: std::make_unique<OpaqueFunctor>(OpaqueFunctor::Construct<Receiver, Functor, Decomposer>(receiverPtr, std::forward<Functor>(functor)))
        }).opaqueFunctor.get();
    }
    return Connection(*this, senderPtr, opaqueFunctor);
}

template<typename Sender, typename Functor>
kF::Meta::Connection kF::Meta::Signal::connect(const Sender &sender, Functor &&functor) noexcept_ndebug
{
    return connect(sender, nullptr, std::forward<Functor>(functor));
}

template<typename Sender, typename ...Args>
void kF::Meta::Signal::emit(const Sender &sender, Args &&...args)
{
    constexpr auto getPtr = [](const auto &input) -> const void * {
        if constexpr (std::is_same_v<std::remove_cvref_t<decltype(input)>, std::nullptr_t>)
            return static_cast<const void *>(input);
        else if constexpr (std::is_pointer_v<std::remove_cvref_t<decltype(input)>>)
            return static_cast<const void *>(input);
        else
            return static_cast<const void *>(&input);
    };

    const void * const senderPtr = getPtr(sender);

    kFAssert(sizeof...(Args) == argsCount(),
        throw std::runtime_error("Meta::Signal::emit: Invalid parameters, given " + std::to_string(sizeof...(Args)) + " but expected " + std::to_string(argsCount()) + " argument(s)"));
    Var arguments[] { Var::Assign(std::forward<Args>(args))... };
    for (const auto &slot : _desc->slots) {
        if (senderPtr != slot.sender || !slot.opaqueFunctor->invokeFunc)
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