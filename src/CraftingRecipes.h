#pragma once

#include "CraftingRecipeSort.h"

#include <cstddef>
#include <map>
#include <string>
#include <vector>

namespace MyGUI
{
class Widget;
}

extern const char kCraftingRecipesPanelName[];
extern const char kCraftingRecipeLineKey[];
extern const char kCraftingRecipeButtonKey[];

struct CraftingRecipeCategory
{
    enum Kind { Other, Weapon, Armor };
    enum { Unknown = -1 };

    CraftingRecipeCategory();

    Kind kind;
    int weaponClass;
    int armorSlot;
    int armorClass;
    bool supportsWeaponStats;
};

struct CraftingCategoryFilter
{
    enum Field { AllRecipes, AllWeapons, WeaponClass, AllArmor, ArmorSlot, ArmorClass };

    CraftingCategoryFilter(Field field = AllRecipes, int value = CraftingRecipeCategory::Unknown);
    bool operator==(const CraftingCategoryFilter& other) const;
    bool Matches(const CraftingRecipeCategory& category) const;

    Field field;
    int value;
};

typedef std::vector<CraftingRecipeCategory> CraftingRecipeCategories;
typedef std::vector<CraftingCategoryFilter> CraftingCategoryOptions;
typedef std::map<std::string, std::string> CraftingCoveragePartOptions;

CraftingCategoryOptions GetCraftingCategoryOptions(const CraftingRecipeCategories& recipes);
CraftingCategoryOptions GetCraftingArmorClassOptions(
    const CraftingRecipeCategories& recipes,
    const CraftingCategoryFilter& categoryFilter);
std::string GetCraftingCategoryLabel(const CraftingCategoryFilter& filter);

void InstallCraftingRecipeHooks();
void ObserveCraftingRecipeUiThread();
unsigned long long GetCraftingRecipeRevision();
long GetResearchRowsRevision();
std::size_t GetAvailableCraftingRecipeCount();
bool TryGetCraftingRecipeCategory(MyGUI::Widget* button, CraftingRecipeCategory& category);
bool TryGetCraftingRecipeSortValue(MyGUI::Widget* button, CraftingRecipeSortStat stat, double& value);
void AddCraftingCoverageParts(MyGUI::Widget* button, CraftingCoveragePartOptions& parts);
bool TryGetCraftingCoveragePercent(MyGUI::Widget* button, const std::string& partId, float& percent);
