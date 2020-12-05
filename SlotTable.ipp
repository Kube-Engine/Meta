/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Slot table
 */

template<typename Receiver, typename Functor>
inline kF::Meta::SlotTable::Slot::Generation kF::Meta::SlotTable::Slot::assign(const void * const receiver, Functor &&functor) noexcept_forward_constructible(decltype(functor))
{
    _receiver = receiver;
    _invokeFunc = [](Var &data, const void * const receiver, Var *arguments) {
        using FunctorType = std::remove_cvref_t<Functor>;
        using Decomposer = Internal::FunctionDecomposerHelper<FunctorType>;

        return Internal::Invoke<Receiver, false, Decomposer>(
            data.as<FunctorType>(),
            receiver,
            arguments,
            Decomposer::IndexSequence
        );
    };
    _data.emplace<Functor>(std::forward<Functor>(functor));
    return _generation;
}

inline bool kF::Meta::SlotTable::Slot::release(const Generation generation)
{
    if (_generation == generation) [[likely]] {
        _data.destruct();
        ++_generation;
        return true;
    } else [[unlikely]]
        return false;
}

inline kF::Var kF::Meta::SlotTable::Slot::invoke(const Generation generation, Var *arguments)
{
    if (_generation == generation) [[likely]]
        return _invokeFunc(_data, _receiver, arguments);
    else [[unlikely]]
        return Var();
}

template<typename Receiver, typename Functor>
inline kF::Meta::SlotTable::Page::IndexAndGeneration kF::Meta::SlotTable::Page::insert(const void * const receiver, Functor &&functor) noexcept_forward_constructible(decltype(functor))
{
    if (_sizeLeft) { // Use unasigned slots
        const Index index = PageSize - _sizeLeft;
        --_sizeLeft;
        const auto generation = _data->at(index).assign<Receiver>(receiver, std::forward<Functor>(functor));
        return PackIndexAndGeneration(index, generation);
    } else { // Re-use free slots
        kFAssert(_freeCount, std::terminate()); // Insert must be called after isInsertable returned true
        --_freeCount;
        const auto index = _freeList.back();
        const auto generation = _data->at(index).assign<Receiver>(receiver, std::forward<Functor>(functor));
        _freeList.pop();
        return PackIndexAndGeneration(index, generation);
    }
}

inline void kF::Meta::SlotTable::Page::remove(const IndexAndGeneration indexAndGeneration)
{
    const auto index = GetIndex(indexAndGeneration);
    const auto generation = GetGeneration(indexAndGeneration);

    if (_data->at(index).release(generation)) [[likely]] {
        ++_freeCount;
        _freeList.push(index);
    }
}
inline kF::Var kF::Meta::SlotTable::Page::invoke(const IndexAndGeneration indexAndGeneration, Var *arguments)
{
    const auto index = GetIndex(indexAndGeneration);
    const auto generation = GetGeneration(indexAndGeneration);

    return _data->at(index).invoke(generation, arguments);
}

inline kF::Meta::SlotTable::SlotTable(void) noexcept
    : _pages(1)
{
}

template<typename Receiver, typename Functor>
inline kF::Meta::SlotTable::OpaqueIndex kF::Meta::SlotTable::insert(const void * const receiver, Functor &&functor) noexcept_forward_constructible(decltype(functor))
{
    constexpr auto Dispatch = [](Page &page, const PageIndex pageIndex, const void * const receiver, auto &&functor) {
        return PackOpaqueIndex(pageIndex, page.insert<Receiver>(receiver, std::forward<Functor>(functor)));
    };

    if (auto &page = _pages[_lastAvailablePage]; page.isInsertable())
        return Dispatch(page, _lastAvailablePage, receiver, std::forward<Functor>(functor));
    for (PageIndex pageIndex = 0; auto &page : _pages) {
        if (!page.isInsertable())
            ++pageIndex;
        else {
            _lastAvailablePage = pageIndex;
            return Dispatch(page, pageIndex, receiver, std::forward<Functor>(functor));
        }
    }
    const auto pageIndex = static_cast<PageIndex>(_pages.size());
    _lastAvailablePage = pageIndex;
    return Dispatch(_pages.emplace_back(), pageIndex, receiver, std::forward<Functor>(functor));
}

inline void kF::Meta::SlotTable::remove(const OpaqueIndex opaqueIndex)
{
    const auto pageIndex = GetPageIndex(opaqueIndex);
    const auto indexAndGeneration = GetIndexAndGeneration(opaqueIndex);

    _pages.at(pageIndex).remove(indexAndGeneration);
}

inline kF::Var kF::Meta::SlotTable::invoke(const OpaqueIndex opaqueIndex, Var *arguments)
{
    const auto pageIndex = GetPageIndex(opaqueIndex);
    const auto indexAndGeneration = GetIndexAndGeneration(opaqueIndex);

    return _pages.at(pageIndex).invoke(indexAndGeneration, arguments);
}