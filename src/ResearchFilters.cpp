#include "ResearchFilters.h"

#include "CraftingCore.h"
#include "CraftingInventoryBinding.h"
#include "CraftingSearchText.h"
#include "CraftingWindowDetection.h"

#include <kenshi/GameData.h>
#include <kenshi/GameWorld.h>
#include <kenshi/Globals.h>
#include <kenshi/PlayerInterface.h>
#include <kenshi/gui/DatapanelGUI.h>
#include <kenshi/gui/DataPanelLine.h>
#include <kenshi/gui/ManagementScreen.h>
#include <mygui/MyGUI_Button.h>
#include <mygui/MyGUI_ListBox.h>
#include <mygui/MyGUI_ScrollView.h>
#include <mygui/MyGUI_TabControl.h>
#include <mygui/MyGUI_TabItem.h>

#include <algorithm>
#include <sstream>

namespace
{
// KenshiLib exports this value-returning method without a ResearchItem definition.
// The 1.0.65 x64 method writes the technology pointer and eight more bytes to
// the caller's result buffer. Only the pointer is needed for the catalog.
struct ResearchItemResult
{
    GameData* technology;
    unsigned char state[8];
};

static_assert(sizeof(ResearchItemResult) == 16, "Unexpected ResearchItem result size");
typedef ResearchItemResult* (*GetResearchItemFn)(Research*, ResearchItemResult*, GameData*);
typedef bool (*IsResearchFinishedFn)(Research*, GameData*);

GetResearchItemFn GetResearchItemReader()
{
    static const GetResearchItemFn function = reinterpret_cast<GetResearchItemFn>(ResolveKenshiLibExport(
        "?getResearchItem@Research@@QEAA?AVResearchItem@@PEAVGameData@@@Z"));
    return function;
}

IsResearchFinishedFn GetResearchFinishedReader()
{
    static const IsResearchFinishedFn function = reinterpret_cast<IsResearchFinishedFn>(ResolveKenshiLibExport(
        "?isFinished@Research@@QEAA_NPEAVGameData@@@Z"));
    return function;
}

Research* GetPlayerResearch()
{
    return ou != 0 && ou->player != 0 ? ou->player->technology : 0;
}

bool CompareResearchEntries(const ResearchEntry& left, const ResearchEntry& right)
{
    if (left.normalizedName != right.normalizedName)
    {
        return left.normalizedName < right.normalizedName;
    }
    return left.category != right.category ? left.category < right.category : left.id < right.id;
}

ManagementScreen* GetResearchScreen(MyGUI::TabItem* tab)
{
    ManagementScreen* screen = ManagementScreen::getSingleton();
    if (screen == 0 || !IsDescendantOf(tab, screen->tabs)
        || !IsDescendantOf(screen->researchCategoriesListBox, tab))
    {
        return 0;
    }
    return screen;
}

void RevealResearchRow(DatapanelGUI* panel, MyGUI::Widget* button)
{
    MyGUI::ScrollView* scroll = panel->scrollWin != 0 ? panel->scrollWin->castType<MyGUI::ScrollView>(false) : 0;
    if (scroll != 0 && IsDescendantOf(button, scroll))
    {
        // MyGUI offsets the canvas toward negative coordinates when scrolling down.
        const MyGUI::IntPoint offset = scroll->getViewOffset();
        const int top = offset.top - (button->getAbsoluteTop() - scroll->getAbsoluteTop());
        scroll->setViewOffset(MyGUI::IntPoint(offset.left, (std::min)(0, top)));
    }
}

void AppendResearchReferences(std::ostream& text, const GameData& definition, const char* key,
    const char* label, int repetition = -1)
{
    const Ogre::vector<GameDataReference>::type* references = definition.getReferenceListIfExists(key);
    if (references == 0)
    {
        return;
    }

    bool headingWritten = false;
    for (Ogre::vector<GameDataReference>::type::const_iterator it = references->begin(); it != references->end(); ++it)
    {
        if (it->sid.empty() || (repetition >= 0 && it->values.value[0] != repetition))
        {
            continue;
        }
        if (!headingWritten)
        {
            text << "\n\n" << label;
            headingWritten = true;
        }
        const GameData* target = ou->gamedata.getData(it->sid);
        text << "\n- " << (target != 0 && !target->name.empty() ? target->name : it->sid);
    }
}
}

const ResearchEntries* GetResearchCatalog(long revision)
{
    const GetResearchItemFn getResearchItem = GetResearchItemReader();
    const IsResearchFinishedFn isFinished = GetResearchFinishedReader();
    Research* research = GetPlayerResearch();
    if (research == 0 || getResearchItem == 0)
    {
        return 0;
    }

    static GameWorld* cachedWorld = 0;
    static Research* cachedResearch = 0;
    static long cachedRevision = 0;
    static ResearchEntries entries;
    if (cachedWorld != ou || cachedResearch != research || cachedRevision != revision)
    {
        entries.clear();
        // Borrow the live record set to avoid allocating or modifying a native research list.
        const ogre_unordered_map<int, GameData*>::type& data = ou->gamedata._getAllData();
        for (ogre_unordered_map<int, GameData*>::type::const_iterator it = data.begin(); it != data.end(); ++it)
        {
            GameData* definition = it->second;
            if (definition == 0 || definition->type != RESEARCH || definition->stringID.empty()
                || ou->gamedata.getData(definition->stringID, RESEARCH) != definition)
            {
                continue;
            }
            const auto category = definition->sdata.find("category");
            const bool hasCategory = category != definition->sdata.end() && !category->second.empty();
            // The engine restores its finished ID set when it loads the save.
            // Repeatable stages have their own native GameData records and IDs.
            const bool learned = isFinished != 0 && isFinished(research, definition);
            ResearchItemResult item = {};
            if (!learned && hasCategory)
            {
                // getAvailableResearch also cleans up the queue. Read each record instead.
                getResearchItem(research, &item, definition);
            }
            if (isFinished != 0 || item.technology == definition)
            {
                ResearchEntry entry;
                entry.id = definition->stringID;
                entry.name = definition->name.empty() ? entry.id : definition->name;
                if (hasCategory)
                {
                    entry.category = category->second;
                }
                entry.normalizedName = NormalizeSearchText(entry.name);
                entry.status = learned ? ResearchEntryLearned
                    : (item.technology == definition ? ResearchEntryAvailable : ResearchEntryUnavailable);
                entries.push_back(entry);
            }
        }
        std::sort(entries.begin(), entries.end(), CompareResearchEntries);
        cachedWorld = ou;
        cachedResearch = research;
        cachedRevision = revision;
    }
    return &entries;
}

bool CanReadLearnedResearch()
{
    return GetResearchItemReader() != 0 && GetResearchFinishedReader() != 0;
}

std::string GetCurrentNativeResearchType(MyGUI::TabItem* tab)
{
    ManagementScreen* screen = GetResearchScreen(tab);
    return screen != 0 ? screen->techCategory : std::string();
}

const char* GetResearchEntryStatusLabel(ResearchEntryStatus status)
{
    switch (status)
    {
    case ResearchEntryAvailable: return "Available";
    case ResearchEntryLearned: return "Learned";
    default: return "Unavailable";
    }
}

std::string DescribeResearchEntry(const ResearchEntry& entry)
{
    const GameData* definition = ou != 0 ? ou->gamedata.getData(entry.id, RESEARCH) : 0;
    if (definition == 0)
    {
        return "Research definition unavailable";
    }

    std::ostringstream text;
    text << entry.name << '\n' << GetResearchEntryStatusLabel(entry.status);
    if (!entry.category.empty())
    {
        text << " | " << entry.category;
    }
    const auto description = definition->sdata.find("description");
    if (description != definition->sdata.end() && !description->second.empty())
    {
        text << "\n\n" << description->second;
    }
    AppendResearchReferences(text, *definition, "requirements", "Prerequisites");

    const auto repeats = definition->idata.find("repeats");
    const auto stage = definition->idata.find("repetition number");
    const int repetition = repeats != definition->idata.end() && repeats->second > 0 && stage != definition->idata.end()
        ? stage->second : -1;
    AppendResearchReferences(text, *definition, "enable buildings", "Buildings unlocked", repetition);
    AppendResearchReferences(text, *definition, "enable item", "Items unlocked", repetition);
    AppendResearchReferences(text, *definition, "enable armour", "Armor unlocked", repetition);
    AppendResearchReferences(text, *definition, "enable weapon type", "Weapon types unlocked", repetition);
    AppendResearchReferences(text, *definition, "enable weapon model", "Weapon models unlocked", repetition);
    AppendResearchReferences(text, *definition, "enable crossbow", "Crossbows unlocked", repetition);
    AppendResearchReferences(text, *definition, "enable backpack", "Backpacks unlocked", repetition);
    AppendResearchReferences(text, *definition, "enable robotics", "Robotics unlocked", repetition);
    AppendResearchReferences(text, *definition, "improve buildings", "Affected buildings");
    return text.str();
}

bool OpenNativeResearchEntry(MyGUI::TabItem* tab, const ResearchEntry& entry)
{
    if (entry.status != ResearchEntryAvailable)
    {
        return false;
    }
    ManagementScreen* screen = GetResearchScreen(tab);
    if (screen == 0 || !tab->getInheritedVisible() || ou == 0
        || ou->gamedata.getData(entry.id, RESEARCH) == 0)
    {
        return false;
    }

    MyGUI::ListBox* types = screen->researchCategoriesListBox;
    std::size_t index = 0;
    for (; index < types->getItemCount(); ++index)
    {
        if (types->getItemNameAt(index).asUTF8() == entry.category)
        {
            break;
        }
    }
    if (index == types->getItemCount())
    {
        return false;
    }

    types->setIndexSelected(index);
    types->beginToItemAt(index);
    if (screen->techCategory != entry.category)
    {
        screen->researchTypeSelect(types, index);
    }

    DatapanelGUI* panel = screen->availableList;
    MyGUI::Widget* scope = FindWidgetInParentByToken(tab, kResearchAvailablePanelName);
    const int category = panel != 0 ? panel->_NV_getCurrentCategory() : -1;
    if (category < 0 || scope == 0 || !scope->getInheritedVisible())
    {
        return false;
    }

    for (int i = 0, count = panel->getNumLines(category); i < count; ++i)
    {
        DataPanelLine* line = panel->getLineByNum(category, i);
        if (line != 0 && line->getType() == DataPanelLine::DPL_RESEARCH)
        {
            MyGUI::Button* button = static_cast<DataPanelLine_Research*>(line)->but;
            if (IsDescendantOf(button, scope) && button->getUserString("id") == entry.id)
            {
                RevealResearchRow(panel, button);
                screen->mouseOverCallback(button, 0, 0);
                return true;
            }
        }
    }
    return false;
}
