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
        const MyGUI::IntPoint offset = scroll->getViewOffset();
        const int top = button->getAbsoluteTop() - scroll->getAbsoluteTop() + offset.top;
        scroll->setViewOffset(MyGUI::IntPoint(offset.left, (std::max)(0, top)));
    }
}
}

const ResearchEntries* GetResearchCatalog(long revision)
{
    static GetResearchItemFn getResearchItem = reinterpret_cast<GetResearchItemFn>(ResolveKenshiLibExport(
        "?getResearchItem@Research@@QEAA?AVResearchItem@@PEAVGameData@@@Z"));
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
            if (definition == 0 || definition->type != RESEARCH || definition->stringID.empty())
            {
                continue;
            }
            const auto category = definition->sdata.find("category");
            if (category == definition->sdata.end() || category->second.empty())
            {
                continue;
            }

            // getAvailableResearch also cleans up the queue. Read each record instead.
            ResearchItemResult item = {};
            getResearchItem(research, &item, definition);
            if (item.technology == definition)
            {
                ResearchEntry entry;
                entry.id = definition->stringID;
                entry.name = definition->name;
                entry.category = category->second;
                entry.normalizedName = NormalizeSearchText(entry.name);
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

bool OpenNativeResearchEntry(MyGUI::TabItem* tab, const ResearchEntry& entry)
{
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
