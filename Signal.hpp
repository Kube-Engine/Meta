/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Meta Signal
 */

#pragma once

#include <memory>

#include "SlotTable.hpp"

/**
 * @brief Signal is used to store meta-data about a free / member / static function
 *
 * Each signal is attached to a class. Signal aren't thread safe.
 * However, each class is separated from others: you can use two classes' signals on two different threads
 */
class kF::Meta::Signal
{
public:
    struct KF_ALIGN_QUARTER_CACHELINE Descriptor
    {
        const Internal::OpaqueFunction signalPtr { nullptr };
        const HashedName name { 0 };
        const std::uint32_t argsCount { 0 };

        template<auto SignalPtr>
        static Descriptor Construct(const HashedName name) noexcept;
    };
    static_assert(sizeof(Descriptor) == Core::CacheLineQuarterSize, "Signal descriptor must take the quarter of a cacheline");

    /** @brief Construct passing a descriptor instance */
    Signal(Descriptor *desc = nullptr) noexcept : _desc(desc) {}

    /** @brief Copy constructor */
    Signal(const Signal &other) noexcept = default;

    /** @brief Copy assignment */
    Signal &operator=(const Signal &other) noexcept = default;

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

    /** @brief Get the unique default slot table */
    [[nodiscard]] static SlotTable &GetSlotTable(void) { return _SlotTable; }

private:
    Descriptor *_desc = nullptr;

    static inline SlotTable _SlotTable {};
};