#pragma once

#include "CraftingRecipes.h"
#include "CraftingRecipeSort.h"

#include <cstddef>
#include <string>

namespace MyGUI
{
class Widget;
class Button;
class EditBox;
class TabItem;
class TextBox;
}

enum SearchFocusHotkeyKind
{
    SearchFocusHotkeyKind_None = 0,
    SearchFocusHotkeyKind_Slash,
    SearchFocusHotkeyKind_CtrlF,
};

struct SearchUiCallbacks
{
    SearchUiCallbacks()
        : onSearchTextChanged(0)
        , onSearchEditFocusChanged(0)
        , onSearchPlaceholderClicked(0)
        , onSearchClearButtonClicked(0)
    {
    }

    void (*onSearchTextChanged)(MyGUI::EditBox* sender);
    void (*onSearchEditFocusChanged)(MyGUI::Widget* oldWidget, MyGUI::Widget* newWidget);
    void (*onSearchPlaceholderClicked)(MyGUI::Widget* sender);
    void (*onSearchClearButtonClicked)(MyGUI::Widget* sender);
};

struct CraftingCategorySelection
{
    CraftingCategorySelection()
        : armorClass(CraftingCategoryFilter::ArmorClass), minimumCoverage(0), sortMode(CraftingRecipeSortOriginal) {}

    CraftingCategoryFilter category;
    CraftingCategoryFilter armorClass;
    std::string coveragePart;
    int minimumCoverage;
    CraftingRecipeSortMode sortMode;
};

MyGUI::Widget* FindControlsContainer();
MyGUI::EditBox* FindSearchEditBox();
MyGUI::TextBox* FindSearchPlaceholderTextBox();
MyGUI::Button* FindSearchClearButton();
MyGUI::TextBox* FindSearchCountTextBox();
MyGUI::Widget* ResolveTraderParentFromControlsContainer();
MyGUI::TabItem* GetResearchSearchTab();
const std::string& GetNormalizedResearchSearchQuery();
void FocusSearchEdit(MyGUI::EditBox* searchEdit, const char* reason);
void FocusSearchEditIfRequested(MyGUI::EditBox* searchEdit, const char* reason);
bool IsSearchEditFocused(MyGUI::EditBox* searchEdit);
void UpdateSearchUiState();
CraftingCategorySelection UpdateCraftingCategoryControls(const CraftingRecipeCategories& recipes, bool canSort,
    const CraftingCoveragePartOptions& parts = CraftingCoveragePartOptions());
void UpdateSearchCountText(
    std::size_t visibleEntryCount,
    std::size_t totalEntryCount,
    std::size_t visibleQuantity);
void TickSearchContainerDrag();
SearchFocusHotkeyKind DetectSearchFocusHotkeyPressedEdge(MyGUI::EditBox* searchEdit);
void ApplyRuntimeSearchUiConfig(const char* reason);
void TickPhase2ControlsScaffold();
