/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Meta Signal
 */

#pragma once

#include <functional>
#include <memory>

#include "Var.hpp"

namespace kF::Meta
{
    struct OpaqueFunctor;
    struct Slot;
    class Connection;
}

/**
 * @brief Signal is used to store meta-data about a free / member / static function
 *
 * Each signal is attached to a class. Signal aren't thread safe.
 * However, each class is separated from others: you can use two classes' signals on two different threads
 */
class kF::Meta::Signal
{
public:
    struct Descriptor
    {
        const Internal::OpaqueFunction signalPtr { nullptr };
        const HashedName name { 0 };
        const std::size_t argsCount { 0 };
        std::vector<Slot> slots {};
        std::vector<std::size_t> freeSlots {};

        template<auto SignalPtr>
        static Descriptor Construct(const HashedName name) noexcept;
    };

    /** @brief Construct passing a descriptor instance */
    Signal(Descriptor *desc = nullptr) noexcept : _desc(desc) {}

    /** @brief Copy constructor */
    Signal(const Signal &other) noexcept = default;

    /** @brief Copy assignment */
    Signal &operator=(const Signal &other) = default;

    /** @brief Fast valid check */
    [[nodiscard]] operator bool(void) const noexcept { return _desc; }

    /** @brief Compare operator */
    [[nodiscard]] bool operator==(const Signal &other) const noexcept { return _desc == other._desc; }
    [[nodiscard]] bool operator!=(const Signal &other) const noexcept { return _desc != other._desc; }

    /** @brief Retreive signal's pointer */
    [[nodiscard]] Internal::OpaqueFunction signalPtr(void) const noexcept { return _desc->signalPtr; }

    /** @brief Retreive signal's name */
    [[nodiscard]] HashedName name(void) const noexcept { return _desc->name; }

    /** @brief Retreive signal's argument count */
    [[nodiscard]] std::size_t argsCount(void) const noexcept { return _desc->argsCount; }

    /** @brief Connect a member slot to signal */
    template<typename Sender, typename Receiver, typename Functor>
    [[nodiscard]] Connection connect(const Sender &sender, const Receiver &receiver, Functor &&functor) noexcept_ndebug;

    /** @brief Connect non-member slot to signal */
    template<typename Sender, typename Functor>
    [[nodiscard]] inline Connection connect(const Sender &sender, Functor &&functor) noexcept_ndebug;

    /** @brief Emit a signal, calling each connected slots */
    template<typename Sender, typename ...Args>
    void emit(const Sender &sender, Args &&...args);

public:
    /**
     * @brief Disconnect a slot though its opaque functor
     *
     * This function should only be used by Connection class
     */
    inline void disconnect(const OpaqueFunctor *opaqueFunctor) noexcept;

private:
    Descriptor *_desc = nullptr;
};

/**
 * @brief OpaqueFunctor stores a callback (either pointer of functor)
 */
struct kF::Meta::OpaqueFunctor
{
    using InvokeFunc = bool(*)(Var &, const void *, Var *);

    const void *receiver { nullptr };
    InvokeFunc invokeFunc { nullptr };
    Var data {};

    /** @brief Constructs the opaque functor of a slot */
    template<typename Receiver, typename Functor, typename Decomposer>
    static OpaqueFunctor Construct(const void *receiver, Functor &&functor) noexcept;
};

/**
 * @brief Slot describes how a signal's callback should be called
 */
struct kF::Meta::Slot
{
    const void *sender { nullptr };
    std::unique_ptr<OpaqueFunctor> opaqueFunctor {};
};

/**
 * @brief Connection store an active slot. When destroy, it will release the slot
 */
class kF::Meta::Connection
{
public:
    /** @brief Construct an empty connection */
    Connection(void) noexcept = default;

    /** @brief Construct an empty connection */
    Connection(const Signal signal, const void *sender, const OpaqueFunctor *opaqueFunctor) noexcept : _signal(signal), _sender(sender), _opaqueFunctor(opaqueFunctor) {}

    /** @brief Construct an empty connection */
    Connection(Connection &&other) noexcept { swap(other); }

    /** @brief Destroy and release a connection */
    ~Connection(void) noexcept {
        if (_signal)
            _signal.disconnect(_opaqueFunctor);
    }

    /** @brief Fast valid check */
    operator bool(void) const noexcept { return _signal; }

    /** @brief Move assignment */
    Connection &operator=(Connection &&other) noexcept { swap(other); return *this; }

    /** @brief Comparison operator */
    [[nodiscard]] bool operator==(const Connection &other) { return _signal == other._signal && _sender == other._sender && _opaqueFunctor == other._opaqueFunctor; }

    /** @brief Swap two connections */
    void swap(Connection &other) noexcept { std::swap(_signal, other._signal); std::swap(_sender, other._sender); std::swap(_opaqueFunctor, other._opaqueFunctor); }

    /** @brief Disconnect underlying signal (will not check if empty) */
    void disconnect(void) noexcept {
        _signal.disconnect(_opaqueFunctor);
        _signal = Signal();
        _sender = nullptr;
        _opaqueFunctor = nullptr;
    }

    /** @brief Retreive connection's signal */
    [[nodiscard]] const Signal signal(void) const noexcept { return _signal; }

    /** @brief Retreive connection's sender */
    [[nodiscard]] const void *sender(void) const noexcept { return _sender; }

    /** @brief Retreive connection's opaque functor */
    [[nodiscard]] const OpaqueFunctor *opaqueFunctor(void) const noexcept { return _opaqueFunctor; }

private:
    Signal _signal { nullptr };
    const void *_sender { nullptr };
    const OpaqueFunctor *_opaqueFunctor { nullptr };
};