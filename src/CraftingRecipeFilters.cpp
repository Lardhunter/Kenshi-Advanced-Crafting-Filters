#include "CraftingRecipes.h"

#include <kenshi/Enums.h>
#include <kenshi/Gear.h>

#include <set>
#include <sstream>

namespace
{
void AppendOptions(CraftingCategoryOptions& options, CraftingCategoryFilter::Field field, const std::set<int>& values)
{
    for (std::set<int>::const_iterator it = values.begin(); it != values.end(); ++it)
    {
        options.push_back(CraftingCategoryFilter(field, *it));
    }
}

std::string NumberedLabel(const char* prefix, int value)
{
    std::ostringstream label;
    label << prefix << ' ' << value;
    return label.str();
}

std::string GetWeaponClassLabel(int value)
{
    switch (value)
    {
    case SKILL_KATANAS: return "Katanas";
    case SKILL_SABRES: return "Sabres";
    case SKILL_HACKERS: return "Hackers";
    case SKILL_HEAVY: return "Heavy Weapons";
    case SKILL_BLUNT: return "Blunt Weapons";
    case ATTACK_POLEARMS: return "Polearms";
    case SKILL_BOW: return "Crossbows";
    case SKILL_TURRET: return "Turrets";
    case SKILL_UNARMED: return "Unarmed";
    default: return NumberedLabel("Weapon Class", value);
    }
}

std::string GetArmorSlotLabel(int value)
{
    switch (value)
    {
    case ATTACH_HAT: return "Head";
    case ATTACH_BODY: return "Chest";
    case ATTACH_LEGS: return "Legs";
    case ATTACH_BOOTS: return "Boots";
    case ATTACH_SHIRT: return "Shirts";
    case ATTACH_GLOVES: return "Gloves";
    case ATTACH_NECK: return "Neck";
    case ATTACH_BELT: return "Belt";
    case ATTACH_BACK: return "Back";
    case ATTACH_BACKPACK: return "Backpack";
    case ATTACH_EYES: return "Eyes";
    case ATTACH_HAIR: return "Hair";
    case ATTACH_BEARD: return "Beard";
    case ATTACH_WEAPON: return "Weapon Slot";
    case ATTACH_LEFT_ARM: return "Left Arm";
    case ATTACH_RIGHT_ARM: return "Right Arm";
    case ATTACH_LEFT_LEG: return "Left Leg";
    case ATTACH_RIGHT_LEG: return "Right Leg";
    default: return NumberedLabel("Equipment Slot", value);
    }
}

const char* GetArmorClassLabel(int value)
{
    switch (value)
    {
    case GEAR_CLOTH: return "Cloth";
    case GEAR_LIGHT: return "Light";
    case GEAR_MEDIUM: return "Medium";
    case GEAR_HEAVY: return "Heavy";
    default: return "All Classes";
    }
}
}

CraftingRecipeCategory::CraftingRecipeCategory()
    : kind(Other)
    , weaponClass(Unknown)
    , armorSlot(Unknown)
    , armorClass(Unknown)
    , supportsWeaponStats(false)
{
}

CraftingCategoryFilter::CraftingCategoryFilter(Field field, int value)
    : field(field)
    , value(value)
{
}

bool CraftingCategoryFilter::operator==(const CraftingCategoryFilter& other) const
{
    return field == other.field && value == other.value;
}

bool CraftingCategoryFilter::Matches(const CraftingRecipeCategory& category) const
{
    switch (field)
    {
    case AllWeapons: return category.kind == CraftingRecipeCategory::Weapon;
    case WeaponClass: return category.kind == CraftingRecipeCategory::Weapon && category.weaponClass == value;
    case AllArmor: return category.kind == CraftingRecipeCategory::Armor;
    case ArmorSlot: return category.kind == CraftingRecipeCategory::Armor && category.armorSlot == value;
    case ArmorClass:
        return value == CraftingRecipeCategory::Unknown
            || (category.kind == CraftingRecipeCategory::Armor && category.armorClass == value);
    case AllRecipes: return true;
    }
    return false;
}

CraftingCategoryOptions GetCraftingCategoryOptions(const CraftingRecipeCategories& recipes)
{
    bool hasWeapons = false;
    bool hasArmor = false;
    std::set<int> weaponClasses;
    std::set<int> armorSlots;
    for (CraftingRecipeCategories::const_iterator it = recipes.begin(); it != recipes.end(); ++it)
    {
        if (it->kind == CraftingRecipeCategory::Weapon)
        {
            hasWeapons = true;
            if (it->weaponClass >= 0)
            {
                weaponClasses.insert(it->weaponClass);
            }
        }
        else if (it->kind == CraftingRecipeCategory::Armor)
        {
            hasArmor = true;
            if (it->armorSlot >= 0)
            {
                armorSlots.insert(it->armorSlot);
            }
        }
    }

    CraftingCategoryOptions options(1, CraftingCategoryFilter());
    if (hasWeapons)
    {
        options.push_back(CraftingCategoryFilter(CraftingCategoryFilter::AllWeapons));
        AppendOptions(options, CraftingCategoryFilter::WeaponClass, weaponClasses);
    }
    if (hasArmor)
    {
        options.push_back(CraftingCategoryFilter(CraftingCategoryFilter::AllArmor));
        AppendOptions(options, CraftingCategoryFilter::ArmorSlot, armorSlots);
    }
    return options;
}

CraftingCategoryOptions GetCraftingArmorClassOptions(
    const CraftingRecipeCategories& recipes,
    const CraftingCategoryFilter& categoryFilter)
{
    std::set<int> classes;
    for (CraftingRecipeCategories::const_iterator it = recipes.begin(); it != recipes.end(); ++it)
    {
        if (it->kind == CraftingRecipeCategory::Armor && it->armorClass >= 0 && categoryFilter.Matches(*it))
        {
            classes.insert(it->armorClass);
        }
    }

    CraftingCategoryOptions options(1, CraftingCategoryFilter(CraftingCategoryFilter::ArmorClass));
    AppendOptions(options, CraftingCategoryFilter::ArmorClass, classes);
    return options;
}

std::string GetCraftingCategoryLabel(const CraftingCategoryFilter& filter)
{
    switch (filter.field)
    {
    case CraftingCategoryFilter::AllWeapons: return "All Weapons";
    case CraftingCategoryFilter::WeaponClass: return GetWeaponClassLabel(filter.value);
    case CraftingCategoryFilter::AllArmor: return "All Armor";
    case CraftingCategoryFilter::ArmorSlot: return GetArmorSlotLabel(filter.value);
    case CraftingCategoryFilter::ArmorClass: return GetArmorClassLabel(filter.value);
    default: return "All Recipes";
    }
}
