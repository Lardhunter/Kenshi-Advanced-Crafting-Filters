#pragma once

#include <string>
#include <vector>

namespace MyGUI
{
class TabItem;
}

enum ResearchTypeFilter
{
    ResearchTypeFilterAutomatic,
    ResearchTypeFilterAll,
    ResearchTypeFilterCurrent
};

struct ResearchEntry
{
    std::string id;
    std::string name;
    std::string category;
    std::string normalizedName;
};

typedef std::vector<ResearchEntry> ResearchEntries;

// The catalog contains copies of the display values. Resolve the game record again on selection.
const ResearchEntries* GetResearchCatalog(long revision);
bool OpenNativeResearchEntry(MyGUI::TabItem* tab, const ResearchEntry& entry);
