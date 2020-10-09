/**
 * @ Author: Matthieu Moinvaziri
 * @ Description: Unit tests of Signal
 */

#include <gtest/gtest.h>

#include <Kube/Meta/SlotTable.hpp>

using namespace kF;

TEST(SlotTable, Pack)
{
    const Meta::SlotTable::PageIndex pageIndex = 39877297;
    const Meta::SlotTable::Page::Index index = 8732;
    const Meta::SlotTable::Slot::Generation generation = 875;

    const auto indexAndGeneration = Meta::SlotTable::Page::PackIndexAndGeneration(index, generation);
    const auto opaqueIndex = Meta::SlotTable::PackOpaqueIndex(pageIndex, indexAndGeneration);

    ASSERT_EQ(Meta::SlotTable::GetPageIndex(opaqueIndex), pageIndex);
    const auto generatedIndexAndGeneration = Meta::SlotTable::GetIndexAndGeneration(opaqueIndex);
    ASSERT_EQ(generatedIndexAndGeneration, indexAndGeneration);
    ASSERT_EQ(Meta::SlotTable::Page::GetIndex(generatedIndexAndGeneration), index);
    ASSERT_EQ(Meta::SlotTable::Page::GetGeneration(generatedIndexAndGeneration), generation);
}

TEST(SlotTable, NoArguments)
{
    Meta::SlotTable table;
    int trigger = 0;

    auto idx = table.insert([&trigger] { ++trigger; });

    ASSERT_TRUE(table.invoke(idx, nullptr));
    ASSERT_EQ(trigger, 1);
    ASSERT_TRUE(table.invoke(idx, nullptr));
    ASSERT_EQ(trigger, 2);
    table.remove(idx);
    ASSERT_FALSE(table.invoke(idx, nullptr));
    ASSERT_EQ(trigger, 2);
}

TEST(SlotTable, Arguments)
{
    Meta::SlotTable table;
    int trigger = 0;

    auto idx = table.insert([&trigger](int x1, int x2) { trigger += x1 - x2; });

    Var args1[2] { 2, 1 };
    Var args2[2] { 1, 2 };
    ASSERT_TRUE(table.invoke(idx, args1));
    ASSERT_EQ(trigger, 1);
    ASSERT_TRUE(table.invoke(idx, args2));
    ASSERT_EQ(trigger, 0);
    table.remove(idx);
    ASSERT_FALSE(table.invoke(idx, args2));
    ASSERT_EQ(trigger, 0);
}

TEST(SlotTable, InsertionStress)
{
    constexpr auto count = Meta::SlotTable::PageSize * 4;

    Meta::SlotTable table;
    std::vector<Meta::SlotTable::OpaqueIndex> indexes;
    int trigger = 0;

    for (auto i = 0u; i < count; ++i) {
        indexes.emplace_back(table.insert([&trigger]{ ++trigger; }));
        if (i == count / 8) {
            for (auto j = 0u; j < count / 16; ++j) {
                table.remove(indexes.front());
                indexes.erase(indexes.begin());
            }
        }
    }
    for (auto i = 0; auto index : indexes) {
        ASSERT_TRUE(table.invoke(index, nullptr));
        ++i;
        ASSERT_EQ(trigger, i);
    }
}