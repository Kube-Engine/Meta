/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Slot table
 */

#pragma once

#include <array>
#include <vector>

#include <Kube/Core/FlatVector.hpp>

#include "Var.hpp"

#ifndef KUBE_META_CONNECTION_TABLE_PAGE_SIZE
# define KUBE_META_CONNECTION_TABLE_PAGE_SIZE 16384ul
#endif

/** @brief SlotTable stores all slots in pages of stable addresses */
class alignas(32) kF::Meta::SlotTable
{
public:
    /** @brief Opaque slot index of the table */
    using PageIndex = std::uint32_t;

    /** @brief Opaque slot index of the table */
    using OpaqueIndex = std::uint64_t;

    /** @brief Slot stores a callback to be called when a signal is emited (either pointer or functor) */
    class KF_ALIGN_CACHELINE Slot
    {
    public:
        /** @brief Signature of the invoke helper */
        using InvokeFunc = Var(*)(Var &data, Var *arguments);

        /** @brief Generation count used to know validity of a slot */
        using Generation = std::uint16_t;

        /** @brief Assign the slot from any functor */
        template<typename Functor>
        [[nodiscard]] Generation assign(Functor &&functor) noexcept_forward_constructible(Functor);

        /** @brief Release the slot */
        [[nodiscard]] bool release(const Generation generation);

        /** @brief Checks if the slot can be safely invoked with a given generation */
        [[nodiscard]] bool isInvokable(const Generation generation) const noexcept { return _generation == generation; }

        /** @brief Invoke the opaque functor */
        [[nodiscard]] Var invoke(const Generation generation, Var *arguments);

    private:
        Var _data {};
        InvokeFunc _invokeFunc { nullptr };
        Generation _generation { 0 };
    };

    /** @brief Determine the page size of the connection table */
    static constexpr std::size_t PageSize = KUBE_META_CONNECTION_TABLE_PAGE_SIZE / sizeof(Slot);

    static_assert(PageSize > 0 && PageSize <= UINT16_MAX, "PageSize must be of range [1, UINT16_MAX]");

    /** @brief A page contains an array of slots */
    class alignas(32) Page
    {
    public:
        /** @brief Array containing all slots */
        using Array = std::array<Slot, PageSize>;

        /** @brief Index of a slot in a page */
        using Index = std::uint16_t;

        /** @brief Packs a slot index and its generation */
        using IndexAndGeneration = std::uint32_t;

        /** @brief Construct a new page */
        Page(void) noexcept : _data(std::make_unique<Array>()) {}

        /** @brief Move constructor */
        Page(Page &&other) noexcept = default;

        /** @brief Destruct the page */
        ~Page(void) noexcept_destructible(Array) = default;

        /** @brief Move assignment */
        Page &operator=(Page &&other) noexcept = default;

        /** @brief Check if the queue has enough space to receive another slot */
        [[nodiscard]] bool isInsertable(void) const noexcept { return _sizeLeft || _freeCount; }

        /** @brief Insert a new slot into the page */
        template<typename Functor>
        [[nodiscard]] IndexAndGeneration insert(Functor &&functor) noexcept_forward_constructible(Functor);

        /** @brief Remove a slot from the page */
        void remove(const IndexAndGeneration indexAndGeneration);

        /** @brief Invoke a slot */
        [[nodiscard]] Var invoke(const IndexAndGeneration indexAndGeneration, Var *arguments);

        /** @brief Pack an index and a generation */
        [[nodiscard]] static IndexAndGeneration PackIndexAndGeneration(const Index index, const Slot::Generation generation) noexcept
            { return (static_cast<IndexAndGeneration>(index) << (sizeof(Index) * 8)) | generation; }

        /** @brief Get slot index from packed value */
        [[nodiscard]] static Index GetIndex(const IndexAndGeneration indexAndGeneration) noexcept
            { return static_cast<Index>(indexAndGeneration >> (sizeof(Index) * 8)); }

        /** @brief Get slot generation from packed value */
        [[nodiscard]] static Slot::Generation GetGeneration(const IndexAndGeneration indexAndGeneration) noexcept
            { return static_cast<Slot::Generation>(indexAndGeneration & (0xFFFF)); }

    private:
        std::unique_ptr<Array> _data {};
        std::size_t _sizeLeft { PageSize };
        std::size_t _freeCount { 0 };
        Core::FlatVector<Index> _freeList;
    };

    static_assert(sizeof(Page) == 32ul, "Page must take 32 bytes");

    /** @brief Construct the table */
    SlotTable(void) noexcept;

    /** @brief Move constructor */
    SlotTable(SlotTable &&other) noexcept = default;

    /** @brief Default destructor */
    ~SlotTable(void) noexcept_destructible(Page) = default;

    /** @brief Move assignment */
    SlotTable &operator=(SlotTable &&other) noexcept = default;

    /** @brief Insert a slot in the table */
    template<typename Functor>
    [[nodiscard]] OpaqueIndex insert(Functor &&functor) noexcept_forward_constructible(Functor);

    /** @brief Remove a slot from the table */
    void remove(const OpaqueIndex opaqueIndex);

    /** @brief Invoke a slot */
    [[nodiscard]] Var invoke(const OpaqueIndex opaqueIndex, Var *arguments);

    [[nodiscard]] static OpaqueIndex PackOpaqueIndex(const PageIndex pageIndex, const Page::IndexAndGeneration indexAndGeneration) noexcept
        { return (static_cast<OpaqueIndex>(pageIndex) << (sizeof(PageIndex) * 8)) | indexAndGeneration; }

    /** @brief Get a page index from an opaque index */
    [[nodiscard]] static PageIndex GetPageIndex(const OpaqueIndex opaqueIndex) noexcept
        { return static_cast<PageIndex>(opaqueIndex >> (sizeof(PageIndex) * 8)); }

    /** @brief Get a packet slot index and generation from an opaque index */
    [[nodiscard]] static Page::IndexAndGeneration GetIndexAndGeneration(const OpaqueIndex opaqueIndex) noexcept
        { return static_cast<Page::IndexAndGeneration>(opaqueIndex & (0xFFFFFFFF)); }

private:
    std::vector<Page> _pages {};
    PageIndex _lastAvailablePage { 0 };
};

static_assert(sizeof(kF::Meta::SlotTable) == 32ul, "SlotTable must take 32 bytes");