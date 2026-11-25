#pragma once

#include <cstddef>
#include <string>
#include <vector>

enum CraftingRecipeSortMode
{
    CraftingRecipeSortOriginal,
    CraftingRecipeSortNameAscending,
    CraftingRecipeSortNameDescending,
    CraftingRecipeSortBluntDescending,
    CraftingRecipeSortBluntAscending,
    CraftingRecipeSortCutDescending,
    CraftingRecipeSortCutAscending,
    CraftingRecipeSortCutEfficiencyDescending,
    CraftingRecipeSortCutEfficiencyAscending,
    CraftingRecipeSortCutDamageDescending,
    CraftingRecipeSortCutDamageAscending,
    CraftingRecipeSortBluntDamageDescending,
    CraftingRecipeSortBluntDamageAscending,
    CraftingRecipeSortArmorPenetrationDescending,
    CraftingRecipeSortArmorPenetrationAscending,
    CraftingRecipeSortCoverageDescending,
    CraftingRecipeSortCoverageAscending,
    CraftingRecipeSortItemValueDescending,
    CraftingRecipeSortItemValueAscending,
    CraftingRecipeSortSellValueDescending,
    CraftingRecipeSortSellValueAscending,
    CraftingRecipeSortTimeAscending,
    CraftingRecipeSortTimeDescending,
    CraftingRecipeSortSellValuePerHourDescending,
    CraftingRecipeSortSellValuePerHourAscending,
    CraftingRecipeSortModeCount,
};

enum CraftingRecipeSortStat
{
    CraftingSortStatNone,
    CraftingSortStatBluntResistance,
    CraftingSortStatCutResistance,
    CraftingSortStatCutEfficiency,
    CraftingSortStatCutDamage,
    CraftingSortStatBluntDamage,
    CraftingSortStatArmorPenetration,
    CraftingSortStatCoverage,
    CraftingSortStatItemValue,
    CraftingSortStatSellValue,
    CraftingSortStatTime,
    CraftingSortStatSellValuePerHour,
    CraftingSortStatCount,
};

struct CraftingRecipeSortRow
{
    CraftingRecipeSortRow(std::size_t rowIndex, const std::string& sortName)
        : index(rowIndex), name(sortName), value(0.0), hasValue(false) {}

    std::size_t index;
    std::string name;
    double value;
    bool hasValue;
};

const char* GetCraftingRecipeSortLabel(CraftingRecipeSortMode mode);
CraftingRecipeSortStat GetCraftingRecipeSortStat(CraftingRecipeSortMode mode);
bool IsCraftingWeaponSortStat(CraftingRecipeSortStat stat);
bool IsCraftingValueOrTimeSortStat(CraftingRecipeSortStat stat);
void SortCraftingRecipeRows(std::vector<CraftingRecipeSortRow>& rows, CraftingRecipeSortMode mode);
