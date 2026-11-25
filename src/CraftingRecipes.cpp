#include "CraftingRecipes.h"

#include "CraftingCore.h"
#include "CraftingSearchText.h"
#include "CraftingWindowDetection.h"

#include <core/Functions.h>
#include <kenshi/GameData.h>
#include <kenshi/GameWorld.h>
#include <kenshi/Gear.h>
#include <kenshi/Globals.h>
#include <kenshi/RootObjectFactory.h>
#include <kenshi/gui/DataPanelLine.h>
#include <kenshi/gui/ManagementScreen.h>
#include <kenshi/util/hand.h>
#include <mygui/MyGUI_Button.h>
#include <mygui/MyGUI_ListBox.h>
#include <mygui/MyGUI_TabControl.h>
#include <mygui/MyGUI_TabItem.h>

#include <Windows.h>

#include <float.h>
#include <map>
#include <sstream>
#include <utility>

const char kCraftingRecipesPanelName[] = "QueueItemsPanel";
const char kCraftingRecipeLineKey[] = "OTCS_CraftingRecipeLine";
const char kCraftingRecipeButtonKey[] = "OTCS_CraftingRecipeButton";

class CraftingBuilding;
class Research;

// This SDK declares CraftingItem without defining it, so v100 cannot include
// CraftingBuilding.h. Mirror only its getAvailableCrafts value type.
class GameDataGroup
{
public:
    GameDataGroup();
    GameDataGroup* _CONSTRUCTOR();
    GameData* g1;
    GameData* g2;
};

namespace
{
static_assert(sizeof(GameDataGroup) == 16, "Unexpected getAvailableCrafts element size");

typedef std::pair<std::string, std::string> RecipeId;
typedef std::map<std::string, std::pair<std::string, int> > ArmorCoverage;

struct RecipeSortValue
{
    RecipeSortValue() : available(false), value(0.0) {}

    bool available;
    double value;
};

struct RecipeRecord
{
    RecipeRecord(const CraftingRecipeCategory& category, itemType outputType)
        : category(category), outputType(outputType), previewChecked(false) {}

    void SetSortValue(CraftingRecipeSortStat stat, double value)
    {
        sortValues[stat].available = _finite(value) != 0;
        sortValues[stat].value = value;
    }

    CraftingRecipeCategory category;
    itemType outputType;
    bool previewChecked;
    RecipeSortValue sortValues[CraftingSortStatCount];
    ArmorCoverage coverage;
};

typedef std::map<RecipeId, RecipeRecord> RecipeCatalog;
// The crafting description passes 40 through the armor grade resolver with
// the quality upgrade flag off. That resolves to the Standard Grade level 40.
const int kStandardGradePreviewLevel = 40;

typedef void (*GetAvailableCraftsFn)(CraftingBuilding*, lektor<GameDataGroup>&);
typedef void (*CreateResearchLineFn)(DataPanelLine_Research*, DatapanelGUI*, float, bool);
typedef void (*UpdateEnabledResearchItemsFn)(Research*);
typedef void (*UpdateManagementScreenFn)(ManagementScreen*);
typedef GameData* (*GetPlayerManufacturerDataFn)();

GetAvailableCraftsFn g_getAvailableCrafts = 0;
CreateResearchLineFn g_createResearchLine = 0;
UpdateEnabledResearchItemsFn g_updateEnabledResearchItems = 0;
UpdateManagementScreenFn g_updateManagementScreen = 0;
GetPlayerManufacturerDataFn g_getPlayerManufacturerData = 0;
volatile LONG g_craftingUiThread = 0;
volatile LONG g_researchRevision = 0;
volatile LONG g_researchRowsRevision = 0;
LONG g_displayedResearchRevision = 0;
RecipeCatalog g_pendingRecipes;
LONG g_pendingResearchRevision = 0;
bool g_havePendingRecipes = false;
RecipeCatalog g_recipes;
unsigned long long g_recipeRevision = 0;
unsigned long long g_lineSequence = 0;

bool IsCraftingUiThread()
{
    return InterlockedCompareExchange(&g_craftingUiThread, 0, 0)
        == static_cast<LONG>(GetCurrentThreadId());
}

LONG GetResearchRevision()
{
    return InterlockedCompareExchange(&g_researchRevision, 0, 0);
}

int ReadCategoryField(const GameData& output, const char* key)
{
    const auto field = output.idata.find(key);
    return field == output.idata.end() ? CraftingRecipeCategory::Unknown : field->second;
}

CraftingRecipeCategory ReadRecipeCategory(const GameData& output)
{
    CraftingRecipeCategory category;
    switch (output.type)
    {
    case WEAPON:
        category.kind = CraftingRecipeCategory::Weapon;
        category.supportsWeaponStats = true;
        category.weaponClass = ReadCategoryField(output, "skill category");
        if (category.weaponClass == ATTACK_NULL || category.weaponClass == NUM_SKILL_TYPES)
        {
            category.weaponClass = CraftingRecipeCategory::Unknown;
        }
        break;
    case CROSSBOW:
        category.kind = CraftingRecipeCategory::Weapon;
        category.weaponClass = SKILL_BOW;
        break;
    case ARMOUR:
        category.kind = CraftingRecipeCategory::Armor;
        category.armorSlot = ReadCategoryField(output, "slot");
        if (category.armorSlot == ATTACH_NONE)
        {
            category.armorSlot = CraftingRecipeCategory::Unknown;
        }
        category.armorClass = ReadCategoryField(output, "class");
        if (category.armorClass < GEAR_CLOTH || category.armorClass >= GEAR_MAX)
        {
            category.armorClass = CraftingRecipeCategory::Unknown;
        }
        break;
    }
    return category;
}

RecipeId ReadRecipeButtonId(MyGUI::Widget* button)
{
    return RecipeId(button->getUserString("id"), button->getUserString("id2"));
}

void ReadArmorCoverage(const GameData& armor, GameDataContainer& data, ArmorCoverage& coverage)
{
    // The armor constructor resolves "part coverage" links as LOCATIONAL_DAMAGE
    // records and converts the first value from a percentage to a probability.
    const std::string coverageList = "part coverage";
    for (int index = 0, count = armor.getListSize(coverageList); index < count; ++index)
    {
        const std::string& partId = armor.getFromList(coverageList, index);
        const GameData* part = data.getData(partId, LOCATIONAL_DAMAGE);
        if (part != 0 && part->type == LOCATIONAL_DAMAGE && !part->stringID.empty())
        {
            coverage[part->stringID] = std::make_pair(
                part->name.empty() ? part->stringID : part->name, armor.getValueFromList(coverageList, index).value[0]);
        }
    }
}

void ReadRecipePreview(GameWorld& world, const RecipeId& id, RecipeRecord& recipe)
{
    recipe.previewChecked = true;
    GameData* output = world.gamedata.getData(id.first, recipe.outputType);
    GameData* variant = id.second.empty() ? 0 : world.gamedata.getData(id.second);
    if (output == 0 || output->type != recipe.outputType || (!id.second.empty() && variant == 0))
    {
        return;
    }

    GameData* factoryData = output;
    const bool isWeapon = output->type == WEAPON;
    const bool isArmor = output->type == ARMOUR;
    const int levelOverride = isArmor ? kStandardGradePreviewLevel : 0;
    if (isWeapon)
    {
        // The weapon description uses PLAYER_WEAPONS and the exact recipe model,
        // with a zero level override so the factory resolves that model's grade.
        if (variant == 0 || variant->type != MATERIAL_SPECS_WEAPON || g_getPlayerManufacturerData == 0)
        {
            return;
        }
        factoryData = g_getPlayerManufacturerData();
        if (factoryData == 0 || factoryData->type != WEAPON_MANUFACTURER)
        {
            return;
        }
    }

    // Match the crafting description's detached preview and cleanup.
    hand previewHandle;
    previewHandle.setNull();
    Item* preview = world.theFactory->createItem(factoryData, previewHandle, output, variant, levelOverride, 0);
    if (preview == 0)
    {
        return;
    }

    // These are the same numeric calls used to display Value, Sell value and
    // Estimated Craft Time. Compare the unrounded hours and the unit prices.
    const int itemValue = preview->getValueSingle(false);
    const int sellValue = preview->getValueSingle(true);
    const float craftHours = preview->getCraftTime();
    if (itemValue >= 0)
    {
        recipe.SetSortValue(CraftingSortStatItemValue, itemValue);
    }
    if (sellValue >= 0)
    {
        recipe.SetSortValue(CraftingSortStatSellValue, sellValue);
    }
    if (_finite(craftHours) && craftHours >= 0.0f)
    {
        recipe.SetSortValue(CraftingSortStatTime, craftHours);
        if (craftHours > 0.0f && sellValue >= 0)
        {
            recipe.SetSortValue(CraftingSortStatSellValuePerHour, static_cast<double>(sellValue) / craftHours);
        }
    }

    if (isWeapon)
    {
        Weapon* weapon = preview->isWeapon();
        const Sword* sword = weapon != 0 ? weapon->isSword() : 0;
        if (sword != 0)
        {
            recipe.SetSortValue(CraftingSortStatCutDamage, sword->cutDamage);
            recipe.SetSortValue(CraftingSortStatBluntDamage, sword->bluntDamage);
            // The game reads penetration from the base weapon record and treats an absent field as zero.
            const auto penetration = output->fdata.find("armour penetration");
            recipe.SetSortValue(CraftingSortStatArmorPenetration,
                penetration != output->fdata.end() ? penetration->second : 0.0f);
        }
    }
    else if (isArmor)
    {
        const Armour* armor = preview->isArmour();
        if (armor != 0)
        {
            recipe.SetSortValue(CraftingSortStatBluntResistance, armor->bluntResistance);
            recipe.SetSortValue(CraftingSortStatCutResistance, armor->cutResistance);
            if (_finite(armor->cutResistance) && armor->cutResistance > 0.0f)
            {
                // The tooltip displays the fraction of resisted cut damage that is not converted to stun.
                recipe.SetSortValue(CraftingSortStatCutEfficiency, 1.0f - armor->cutToStun);
            }
        }
    }
    world.destroy(preview, false, "Crafting filters equipment preview");
}

void OnGetAvailableCrafts(CraftingBuilding* station, lektor<GameDataGroup>& available)
{
    const LONG researchRevision = GetResearchRevision();
    g_getAvailableCrafts(station, available);
    if (!IsCraftingUiThread())
    {
        return;
    }

    RecipeCatalog recipes;
    if (available.valid() && available.size() <= available.capacity())
    {
        for (std::size_t i = 0; i < available.size(); ++i)
        {
            const GameDataGroup& recipe = available[static_cast<uint32_t>(i)];
            if (recipe.g1 != 0 && !recipe.g1->stringID.empty())
            {
                const RecipeId id(recipe.g1->stringID, recipe.g2 != 0 ? recipe.g2->stringID : "");
                RecipeRecord record(ReadRecipeCategory(*recipe.g1), recipe.g1->type);
                if (record.category.kind == CraftingRecipeCategory::Armor && ou != 0)
                {
                    ReadArmorCoverage(*recipe.g1, ou->gamedata, record.coverage);
                }
                recipes.insert(std::make_pair(id, record));
            }
        }
    }
    g_pendingRecipes.swap(recipes);
    g_pendingResearchRevision = researchRevision;
    g_havePendingRecipes = true;
    ++g_recipeRevision;
}

void OnUpdateEnabledResearchItems(Research* research)
{
    g_updateEnabledResearchItems(research);
    InterlockedIncrement(&g_researchRevision);
    InterlockedIncrement(&g_researchRowsRevision);
}

void OnUpdateManagementScreen(ManagementScreen* screen)
{
    g_updateManagementScreen(screen);
    if (!IsCraftingUiThread() || !TraderState().core.g_controlsEnabled)
    {
        return;
    }

    const LONG revision = GetResearchRevision();
    if (revision == g_displayedResearchRevision || screen->tabs == 0
        || !screen->tabs->getInheritedVisible() || (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0)
    {
        return;
    }

    const int tabIndex = screen->getCurrentTab();
    MyGUI::TabItem* tab = tabIndex >= 0 ? screen->getTab(tabIndex) : 0;
    if (tab == 0 || !tab->getInheritedVisible()
        || !ContainsAsciiCaseInsensitive(tab->getName(), "CraftTab")
        || FindNamedDescendantByTokenRecursive(tab, kCraftingRecipesPanelName, true) == 0)
    {
        return;
    }

    MyGUI::Widget* stations = FindNamedDescendantByTokenRecursive(tab, "CraftingStationsList", true);
    MyGUI::ListBox* stationList = stations != 0 ? stations->castType<MyGUI::ListBox>(false) : 0;
    if (stationList == 0 || stationList->getIndexSelected() >= stationList->getItemCount())
    {
        return;
    }

    // The game's Crafting tab refresh retains the selected station handle and
    // rebinds its existing queue. Run it once for each observed research update.
    g_displayedResearchRevision = revision;
    screen->tabCallback(screen->tabs, static_cast<std::size_t>(tabIndex));
}

bool IsInNamedPanel(MyGUI::Widget* widget, const char* panelName)
{
    for (; widget != 0; widget = widget->getParent())
    {
        if (ContainsAsciiCaseInsensitive(widget->getName(), panelName))
        {
            return true;
        }
    }
    return false;
}

void OnCreateResearchLine(DataPanelLine_Research* line, DatapanelGUI* panel, float top, bool lastLine)
{
    g_createResearchLine(line, panel, top, lastLine);
    if (!IsCraftingUiThread() || line->but == 0)
    {
        return;
    }
    if (!IsInNamedPanel(line->but, kCraftingRecipesPanelName))
    {
        if (IsInNamedPanel(line->but, kResearchAvailablePanelName))
        {
            InterlockedIncrement(&g_researchRowsRevision);
        }
        return;
    }

    if (g_havePendingRecipes)
    {
        g_recipes.swap(g_pendingRecipes);
        g_pendingRecipes.clear();
        g_displayedResearchRevision = g_pendingResearchRevision;
        g_havePendingRecipes = false;
    }

    // The game writes id and id2 to this button after the line is created.
    // Tag the line's other widgets so the filter can move it as a unit.
    std::ostringstream tag;
    tag << ++g_lineSequence;
    const std::string lineId = tag.str();
    for (int i = 0, count = line->getNumWidgets(); i < count; ++i)
    {
        MyGUI::Widget* widget = line->getWidget(i);
        if (widget != 0)
        {
            widget->setUserString(kCraftingRecipeLineKey, lineId);
        }
    }
    line->but->setUserString(kCraftingRecipeLineKey, lineId);
    line->but->setUserString(kCraftingRecipeButtonKey, "1");
    ++g_recipeRevision;
}

}

long GetResearchRowsRevision()
{
    return InterlockedCompareExchange(&g_researchRowsRevision, 0, 0);
}

void InstallCraftingRecipeHooks()
{
    const intptr_t getCrafts = ResolveKenshiLibExport(
        "?getAvailableCrafts@CraftingBuilding@@QEAAXAEAV?$lektor@VGameDataGroup@@@@@Z");
    const intptr_t createLine = KenshiLib::GetRealAddress(&DataPanelLine_Research::_NV_createMe);
    if (getCrafts == 0 || createLine == 0
        || KenshiLib::SUCCESS != KenshiLib::AddHook(getCrafts, OnGetAvailableCrafts, &g_getAvailableCrafts)
        || KenshiLib::SUCCESS != KenshiLib::AddHook(createLine, OnCreateResearchLine, &g_createResearchLine))
    {
        LogWarnLine("Crafting recipe hooks unavailable. The crafting panel will use text search.");
        return;
    }
    LogInfoLine("Crafting recipe category hooks installed.");

    g_getPlayerManufacturerData = reinterpret_cast<GetPlayerManufacturerDataFn>(ResolveKenshiLibExport(
        "?playerManufacturerData@CraftingBuilding@@SAPEAVGameData@@XZ"));
    if (g_getPlayerManufacturerData == 0)
    {
        LogWarnLine("Weapon preview unavailable. Weapon stats will sort as unknown.");
    }

    const intptr_t updateResearch = ResolveKenshiLibExport("?_updateTheEnabledItemsList@Research@@QEAAXXZ");
    const intptr_t updateScreen = KenshiLib::GetRealAddress(&ManagementScreen::update);
    if (updateResearch == 0 || updateScreen == 0
        || KenshiLib::SUCCESS != KenshiLib::AddHook(updateResearch, OnUpdateEnabledResearchItems, &g_updateEnabledResearchItems)
        || KenshiLib::SUCCESS != KenshiLib::AddHook(updateScreen, OnUpdateManagementScreen, &g_updateManagementScreen))
    {
        LogWarnLine("Live research refresh unavailable. Recipe filters will refresh on the next native list update.");
    }
    else
    {
        LogInfoLine("Live research refresh for the crafting tab installed.");
    }
}

void ObserveCraftingRecipeUiThread()
{
    InterlockedCompareExchange(&g_craftingUiThread, static_cast<LONG>(GetCurrentThreadId()), 0);
}

unsigned long long GetCraftingRecipeRevision()
{
    return g_recipeRevision;
}

std::size_t GetAvailableCraftingRecipeCount()
{
    return g_recipes.size();
}

bool TryGetCraftingRecipeCategory(MyGUI::Widget* button, CraftingRecipeCategory& category)
{
    const RecipeCatalog::const_iterator recipe = g_recipes.find(ReadRecipeButtonId(button));
    if (recipe == g_recipes.end())
    {
        return false;
    }

    category = recipe->second.category;
    return true;
}

bool TryGetCraftingRecipeSortValue(MyGUI::Widget* button, CraftingRecipeSortStat stat, double& value)
{
    if (button == 0 || stat <= CraftingSortStatNone || stat >= CraftingSortStatCount
        || stat == CraftingSortStatCoverage || !IsCraftingUiThread())
    {
        return false;
    }

    const RecipeId id = ReadRecipeButtonId(button);
    RecipeCatalog::iterator found = g_recipes.find(id);
    if (found == g_recipes.end())
    {
        return false;
    }
    RecipeRecord& recipe = found->second;
    const bool applicable = IsCraftingValueOrTimeSortStat(stat)
        || (IsCraftingWeaponSortStat(stat) ? recipe.category.supportsWeaponStats
            : recipe.category.kind == CraftingRecipeCategory::Armor);
    if (!applicable)
    {
        return false;
    }
    if (!recipe.previewChecked && ou != 0 && ou->theFactory != 0)
    {
        ReadRecipePreview(*ou, id, recipe);
    }
    value = recipe.sortValues[stat].value;
    return recipe.sortValues[stat].available;
}

void AddCraftingCoverageParts(MyGUI::Widget* button, CraftingCoveragePartOptions& parts)
{
    const RecipeCatalog::const_iterator found = g_recipes.find(ReadRecipeButtonId(button));
    if (found != g_recipes.end())
    {
        const ArmorCoverage& coverage = found->second.coverage;
        for (ArmorCoverage::const_iterator part = coverage.begin(); part != coverage.end(); ++part)
        {
            parts.insert(std::make_pair(part->first, part->second.first));
        }
    }
}

bool TryGetCraftingCoveragePercent(MyGUI::Widget* button, const std::string& partId, float& percent)
{
    const RecipeCatalog::const_iterator found = g_recipes.find(ReadRecipeButtonId(button));
    if (found == g_recipes.end() || found->second.category.kind != CraftingRecipeCategory::Armor)
    {
        return false;
    }
    const ArmorCoverage& coverage = found->second.coverage;
    const ArmorCoverage::const_iterator part = coverage.find(partId);
    percent = part != coverage.end() ? static_cast<float>(part->second.second) : 0.0f;
    return true;
}
