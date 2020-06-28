/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Meta signal
 */

#include "Signal.hpp"

template<auto SignalPtr>
kF::Meta::Signal::Descriptor kF::Meta::Signal::Descriptor::Construct(const HashedName name, std::vector<HashedName> &&arguments) noexcept_ndebug
{
    using Decomposer = Internal::FunctionDecomposerHelper<decltype(SignalPtr)>;

    kFAssert(std::tuple_size_v<typename Decomposer::ArgsTuple> == arguments.size(),
        throw std::runtime_error("Meta::Signal: Invalid number of arguments for given signal signature"));
    return Descriptor {
        .signalPtr = Internal::GetFunctionIdentifier<SignalPtr>(),
        .name = name,
        .arguments = std::move(arguments)
    };
}

template<typename Receiver, typename Functor, typename Decomposer>
kF::Meta::Slot::OpaqueFunctor kF::Meta::Slot::OpaqueFunctor::Construct(const Receiver *receiver, Functor &&functor, const ConnectionType connectionType)
{
    return OpaqueFunctor {
        .threadId = std::this_thread::get_id(),
        .invokeFunc = [](Var &data, const void *receiver, Var *args) {
            using FunctorType = std::remove_cvref_t<Functor>;

            // constexpr bool IsMetaCall = std::is_same_v<typename Decomposer::ArgsTuple, std::tuple<Var *>>;

            auto &func = data.as<FunctorType>();
            if constexpr (false) {
                if constexpr (Decomposer::IsFunctor)
                    func(args);
                else
                    (*func)(args);
            } else
                Internal::Invoke<Receiver, Decomposer, Functor>(func, receiver, args, Decomposer::IndexSequence);
        },
        .data = std::forward<Functor>(functor),
        .connectionType = connectionType
    };
}

template<typename Sender, typename Receiver, typename Functor>
kF::Meta::Connection kF::Meta::Signal::connect(const Sender *sender, const Receiver *receiver, Functor &&functor, const ConnectionType connectionType)
{
    using Decomposer = Internal::FunctionDecomposerHelper<std::remove_cvref_t<Functor>>;

    auto lock = std::lock_guard(_desc->slotsMutex);

    if (_desc->freeSlots.empty()) {
        _desc->slots.emplace_back(Slot {
            .sender = reinterpret_cast<const void *>(sender),
            .receiver = reinterpret_cast<const void *>(receiver),
            .opaqueFunctor = std::make_shared<Slot::OpaqueFunctor>(Slot::OpaqueFunctor::Construct<Receiver, Functor, Decomposer>(receiver, std::forward<Functor>(functor), connectionType))
        });
   } else
        *_desc->slots[_desc->freeSlots.back()].opaqueFunctor = Slot::OpaqueFunctor::Construct<Receiver, Functor, Decomposer>(receiver, std::forward<Functor>(functor), connectionType);
    return Connection(*this, reinterpret_cast<const void *>(sender), reinterpret_cast<const void *>(receiver));
}

template<typename Sender, typename Functor>
kF::Meta::Connection kF::Meta::Signal::connect(const Sender *sender, Functor &&functor, const ConnectionType connectionType)
{
    return connect(sender, static_cast<const void *>(nullptr), std::forward<Functor>(functor), connectionType);
}

template<typename Sender, typename Receiver>
void kF::Meta::Signal::disconnect(const Sender *sender, const Receiver *receiver)
{
    auto lock = std::lock_guard(_desc->slotsMutex);
    auto it = std::find_if(_desc->slots.begin(), _desc->slots.end(), [sender, receiver](const auto &slot) {
        return slot.sender == sender && slot.receiver == receiver;
    });
    if (it == _desc->slots.end())
        return;
    _desc->slots.erase(it);
}

template<typename ...Args>
void kF::Meta::Signal::emit(const void *sender, Args &&...args)
{
    kFAssert(sizeof...(Args) == argsCount(),
        throw std::runtime_error("Meta::Signal::emit: Invalid parameters, given " + std::to_string(sizeof...(Args))
                            + " but expected " + std::to_string(argsCount()) + " argument(s)"));
    Var arguments[] { Var::Assign(std::forward<Args>(args))... };
    auto lock = std::shared_lock(_desc->slotsMutex);
    for (auto &slot : _desc->slots) {
        if (sender == slot.sender)
            slot.opaqueFunctor->invokeFunc(slot.opaqueFunctor->data, slot.receiver, arguments);
    }
}