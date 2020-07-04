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
kF::Meta::Slot::OpaqueFunctor kF::Meta::Slot::OpaqueFunctor::Construct(const Receiver *receiver, Functor &&functor, const ConnectionType connectionType) noexcept_ndebug
{
    return OpaqueFunctor {
        .threadId = std::this_thread::get_id(),
        .invokeFunc = [](Var &data, const void *receiver, Var *args) -> bool {
            using FunctorType = std::remove_cvref_t<Functor>;

            return Internal::Invoke<Receiver, Decomposer, Functor>(data.as<FunctorType>(), receiver, args, Decomposer::IndexSequence).operator bool();
        },
        .data = std::forward<Functor>(functor),
        .connectionType = connectionType
    };
}

template<typename Sender, typename Receiver, typename Functor>
kF::Meta::Connection kF::Meta::Signal::connect(const Sender *sender, const Receiver *receiver, Functor &&functor, const ConnectionType connectionType) noexcept_ndebug
{
    using Decomposer = Internal::FunctionDecomposerHelper<std::remove_cvref_t<Functor>>;

    kFAssert(argsCount() == std::tuple_size_v<typename Decomposer::ArgsTuple>,
        throw std::runtime_error("Meta::Signal::connect: Invalid number of arguments of slot"));

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
kF::Meta::Connection kF::Meta::Signal::connect(const Sender *sender, Functor &&functor, const ConnectionType connectionType) noexcept_ndebug
{
    return connect(sender, static_cast<const void *>(nullptr), std::forward<Functor>(functor), connectionType);
}

template<typename Sender, typename Receiver>
void kF::Meta::Signal::disconnect(const Sender *sender, const Receiver *receiver) noexcept
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
        throw std::runtime_error("Meta::Signal::emit: Invalid parameters, given " + std::to_string(sizeof...(Args)) + " but expected " + std::to_string(argsCount()) + " argument(s)"));
    Var arguments[] { Var::Assign(std::forward<Args>(args))... };
    std::shared_ptr<Var[]> sharedArguments;
    const auto threadId = std::this_thread::get_id();
    auto lock = std::shared_lock(_desc->slotsMutex);
    for (auto &slot : _desc->slots) {
        if (sender != slot.sender)
            continue;
        if (threadId == slot.opaqueFunctor->threadId) {
            if (!slot.opaqueFunctor->invokeFunc(slot.opaqueFunctor->data, slot.receiver, arguments))
                throw std::runtime_error("Meta::Signal::emit: Invalid slot signature");
            continue;
        }
        if (!sharedArguments) {
            sharedArguments = std::make_unique<Var[]>(sizeof...(Args));
            for (auto *ptr = sharedArguments.get(); const auto &arg : arguments) {
                *ptr = arg;
                ++ptr;
            }
        }
        if (auto holder = _DelayedSlotMap.find(slot.opaqueFunctor->threadId); holder)
            holder.value().emplace_back(slot, sharedArguments);
        else
            _DelayedSlotMap.insert(slot.opaqueFunctor->threadId, std::vector<DelayedSlot> { DelayedSlot(slot, sharedArguments) });
    }
}