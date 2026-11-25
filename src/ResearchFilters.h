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

enum ResearchStatusFilter
{
    ResearchStatusFilterAvailable,
    ResearchStatusFilterLearned,
    ResearchStatusFilterAll
};

enum ResearchEntryStatus
{
    ResearchEntryUnavailable,
    ResearchEntryAvailable,
    ResearchEntryLearned
};

struct ResearchEntry
{
    std::string id;
    std::string name;
    std::string category;
    std::string normalizedName;
    ResearchEntryStatus status;
};

typedef std::vector<ResearchEntry> ResearchEntries;

// The catalog contains copies of the display values. Resolve the game record again on selection.
const ResearchEntries* GetResearchCatalog(long revision);
bool CanReadLearnedResearch();
std::string GetCurrentNativeResearchType(MyGUI::TabItem* tab);
const char* GetResearchEntryStatusLabel(ResearchEntryStatus status);
std::string DescribeResearchEntry(const ResearchEntry& entry);
bool OpenNativeResearchEntry(MyGUI::TabItem* tab, const ResearchEntry& entry);
