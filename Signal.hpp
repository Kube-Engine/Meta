/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Meta Signal
 */

#pragma once

#include <shared_mutex>
#include <functional>
#include <thread>

#include <Kube/Core/SafeAccessTable.hpp>

#include "Var.hpp"

/**
 * @brief Signal is used to store meta-data about a free / member / static function
 */
class kF::Meta::Signal
{
public:
    struct Descriptor
    {
        const Internal::OpaqueFunction signalPtr { nullptr };
        const HashedName name { 0 };
        const std::vector<HashedName> arguments {};
        std::vector<Slot> slots {};
        std::vector<std::size_t> freeSlots {};
        std::shared_mutex slotsMutex {};

        template<auto SignalPtr>
        static Descriptor Construct(const HashedName name, std::vector<HashedName> &&names) noexcept_ndebug;
    };

    using DelayedSlotMap = SafeAccessTable<std::thread::id, std::vector<DelayedSlot>>;

    static void ProcessDelayedSlots(void);

    Signal(Descriptor *desc = nullptr) noexcept : _desc(desc) {}
    Signal(const Signal &other) noexcept = default;
    Signal &operator=(const Signal &other) = default;
    [[nodiscard]] operator bool(void) const noexcept { return _desc; }
    [[nodiscard]] bool operator==(const Signal &other) const noexcept { return _desc == other._desc; }
    [[nodiscard]] bool operator!=(const Signal &other) const noexcept { return _desc != other._desc; }

    [[nodiscard]] Internal::OpaqueFunction signalPtr(void) const noexcept { return _desc->signalPtr; }

    [[nodiscard]] HashedName name(void) const noexcept { return _desc->name; }

    [[nodiscard]] std::size_t argsCount(void) const noexcept { return _desc->arguments.size(); }

    [[nodiscard]] const std::vector<HashedName> &arguments(void) const noexcept { return _desc->arguments; }

    template<typename Sender, typename Receiver, typename Functor>
    [[nodiscard]] Connection connect(const Sender *sender, const Receiver *receiver, Functor &&functor, const ConnectionType connectionType = ConnectionType::Safe) noexcept_ndebug;

    template<typename Sender, typename Functor>
    [[nodiscard]] Connection connect(const Sender *sender, Functor &&functor, const ConnectionType connectionType = ConnectionType::Safe) noexcept_ndebug;

    template<typename Sender, typename Receiver>
    void disconnect(const Sender *sender, const Receiver *receiver) noexcept;

    template<typename ...Args>
    void emit(const void *sender, Args &&...args);

private:
    static DelayedSlotMap _DelayedSlotMap;

    Descriptor *_desc = nullptr;
};

struct kF::Meta::Slot
{
    struct OpaqueFunctor
    {
        using InvokeFunc = bool(*)(Var &, const void *, Var *);

        std::thread::id threadId {};
        InvokeFunc invokeFunc { nullptr };
        Var data {};
        ConnectionType connectionType { ConnectionType::Safe };

        template<typename Receiver, typename Functor, typename Decomposer>
        static OpaqueFunctor Construct(const Receiver *receiver, Functor &&functor, const Meta::ConnectionType connectionType) noexcept_ndebug;
    };

    const void *sender { nullptr };
    const void *receiver { nullptr };
    std::shared_ptr<OpaqueFunctor> opaqueFunctor {};
};

struct kF::Meta::DelayedSlot : public Slot
{
    DelayedSlot(const Slot &slot = Slot(), const std::shared_ptr<Var[]> &arguments_ = std::shared_ptr<Var[]>()) : Slot(slot), arguments(arguments_) {}

    std::shared_ptr<Var[]> arguments {};
};

class kF::Meta::Connection
{
public:
    Connection(void) noexcept = default;
    Connection(const Signal signal, const void *sender, const void *receiver) noexcept : _signal(signal), _sender(sender), _receiver(receiver) {}
    Connection(Connection &&other) noexcept { swap(other); }
    ~Connection(void) noexcept {
        if (_signal)
            _signal.disconnect(_sender, _receiver);
    }

    operator bool(void) const noexcept { return _signal; }

    Connection &operator=(Connection &&other) noexcept { swap(other); return *this; }

    void swap(Connection &other) noexcept { std::swap(_signal, other._signal); std::swap(_sender, other._sender); std::swap(_receiver, other._receiver); }

    void disconnect(void) noexcept {
        _signal.disconnect(_sender, _receiver);
        _signal = Signal();
        _sender = nullptr;
        _receiver = nullptr;
    }

private:
    Signal _signal { nullptr };
    const void *_sender { nullptr };
    const void *_receiver { nullptr };
};