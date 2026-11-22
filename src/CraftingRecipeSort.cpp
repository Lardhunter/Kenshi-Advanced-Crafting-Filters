#include "CraftingRecipeSort.h"

#include <algorithm>

namespace
{
struct RecipeSortOption
{
    const char* label;
    CraftingRecipeSortStat stat;
    bool descending;
};

const RecipeSortOption kSortOptions[] =
{
    { "Original Order", CraftingSortStatNone, false },
    { "Name A\xE2\x80\x93Z", CraftingSortStatNone, false },
    { "Name Z\xE2\x80\x93" "A", CraftingSortStatNone, true },
    { "Blunt resistance (high)", CraftingSortStatBluntResistance, true },
    { "Blunt resistance (low)", CraftingSortStatBluntResistance, false },
    { "Cut resistance (high)", CraftingSortStatCutResistance, true },
    { "Cut resistance (low)", CraftingSortStatCutResistance, false },
    { "Cut efficiency (high)", CraftingSortStatCutEfficiency, true },
    { "Cut efficiency (low)", CraftingSortStatCutEfficiency, false },
    { "Cutting damage (high)", CraftingSortStatCutDamage, true },
    { "Cutting damage (low)", CraftingSortStatCutDamage, false },
    { "Blunt damage (high)", CraftingSortStatBluntDamage, true },
    { "Blunt damage (low)", CraftingSortStatBluntDamage, false },
    { "Armor penetration (high)", CraftingSortStatArmorPenetration, true },
    { "Armor penetration (low)", CraftingSortStatArmorPenetration, false },
    { "Coverage (high)", CraftingSortStatCoverage, true },
    { "Coverage (low)", CraftingSortStatCoverage, false },
};
static_assert(sizeof(kSortOptions) / sizeof(kSortOptions[0]) == CraftingRecipeSortModeCount, "Sort options must match the mode enum");

const RecipeSortOption& GetSortOption(CraftingRecipeSortMode mode)
{
    return kSortOptions[mode >= CraftingRecipeSortOriginal && mode < CraftingRecipeSortModeCount
        ? mode : CraftingRecipeSortOriginal];
}

struct RecipeNameOrder
{
    explicit RecipeNameOrder(bool descending) : descending(descending) {}

    bool operator()(const CraftingRecipeSortRow& left, const CraftingRecipeSortRow& right) const
    {
        if (left.name.empty() != right.name.empty())
        {
            return !left.name.empty();
        }
        return descending ? right.name < left.name : left.name < right.name;
    }

    bool descending;
};

struct RecipeStatOrder
{
    explicit RecipeStatOrder(bool descending) : descending(descending) {}

    bool operator()(const CraftingRecipeSortRow& left, const CraftingRecipeSortRow& right) const
    {
        if (left.hasValue != right.hasValue)
        {
            return left.hasValue;
        }
        if (!left.hasValue)
        {
            return false;
        }
        return descending ? right.value < left.value : left.value < right.value;
    }

    bool descending;
};
}

const char* GetCraftingRecipeSortLabel(CraftingRecipeSortMode mode)
{
    return GetSortOption(mode).label;
}

CraftingRecipeSortStat GetCraftingRecipeSortStat(CraftingRecipeSortMode mode)
{
    return GetSortOption(mode).stat;
}

bool IsCraftingWeaponSortStat(CraftingRecipeSortStat stat)
{
    return stat == CraftingSortStatCutDamage || stat == CraftingSortStatBluntDamage
        || stat == CraftingSortStatArmorPenetration;
}

void SortCraftingRecipeRows(std::vector<CraftingRecipeSortRow>& rows, CraftingRecipeSortMode mode)
{
    const RecipeSortOption& option = GetSortOption(mode);
    if (mode == CraftingRecipeSortNameAscending || mode == CraftingRecipeSortNameDescending)
    {
        std::stable_sort(rows.begin(), rows.end(), RecipeNameOrder(option.descending));
    }
    else if (option.stat != CraftingSortStatNone)
    {
        std::stable_sort(rows.begin(), rows.end(), RecipeStatOrder(option.descending));
    }
}
