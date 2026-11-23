#include "CraftingSearchUi.h"

#include "CraftingCore.h"
#include "CraftingDiagnostics.h"
#include "CraftingInventoryBinding.h"
#include "CraftingSearchInputBehavior.h"
#include "CraftingSearchPipeline.h"
#include "CraftingSearchText.h"
#include "CraftingWindowDetection.h"

#include <kenshi/Globals.h>
#include <kenshi/InputHandler.h>

#include <mygui/MyGUI_Button.h>
#include <mygui/MyGUI_ComboBox.h>
#include <mygui/MyGUI_EditBox.h>
#include <mygui/MyGUI_Gui.h>
#include <mygui/MyGUI_ILayer.h>
#include <mygui/MyGUI_InputManager.h>
#include <mygui/MyGUI_LayerManager.h>
#include <mygui/MyGUI_RenderManager.h>
#include <mygui/MyGUI_ResourceManager.h>
#include <mygui/MyGUI_TabControl.h>
#include <mygui/MyGUI_TabItem.h>
#include <mygui/MyGUI_TextBox.h>
#include <mygui/MyGUI_Widget.h>
#include <mygui/MyGUI_Window.h>
#include <mygui/MyGUI_WidgetManager.h>
#include <ois/OISKeyboard.h>

#include <Windows.h>

#include <algorithm>
#include <cstdlib>
#include <sstream>

namespace
{
const char* kControlsContainerName = "OTCS_ControlsContainer";
const char* kSearchEditName = "OTCS_SearchEdit";
const char* kSearchPlaceholderName = "OTCS_SearchPlaceholder";
const char* kSearchClearButtonName = "OTCS_SearchClearButton";
const char* kSearchDragHandleName = "OTCS_SearchDragHandle";
const char* kSearchCountTextName = "OTCS_SearchCountText";
const char* kCraftingCategoryComboName = "OTCS_RecipeCategory";
const char* kCraftingArmorClassComboName = "OTCS_RecipeArmorClass";
const char* kCraftingSortComboName = "OTCS_RecipeSort";
const char* kCoveragePartComboName = "OTCS_CoveragePart";
const char* kCraftingCategoryLabelName = "OTCS_CategoryLabel";
const char* kCraftingArmorClassLabelName = "OTCS_ArmorClassLabel";
const char* kCraftingSortLabelName = "OTCS_SortLabel";
const char* kCoveragePartLabelName = "OTCS_CoveragePartLabel";
const char* kComboTextMeasureName = "OTCS_ComboTextMeasure";
const char* kCoveragePercentName = "OTCS_CoveragePercent";
const char* kCoveragePercentLabelName = "OTCS_CoveragePercentLabel";
const char* kCraftingFilterContentName = "OTCS_FilterContent";
const char* kCraftingCollapseButtonName = "OTCS_CollapseFilters";
const char* kCraftingResizeGripName = "OTCS_ResizeFilters";
const char* kCraftingHeaderName = "OTCS_FilterHeader";
const int kPanelPadding = 8;
const int kPanelGap = 6;
const int kPanelDefaultWidth = 350;
const int kPanelMinWidth = 290;
const int kFilterLabelWidth = 63;
// Space outside the text in the stock dropdown client and list (including the scrollbar).
const int kComboTextPadding = 52;
const int kPanelMinBodyHeight = 127; // Fixed top and bottom sections of Kenshi_GenericWindowSkin.
const int kPanelHeaderHeight = 38;
const int kCollapsedPanelWidth = 190;
const int kSearchCountWidth = 96;
const int kResizeGripSize = 22;

struct FloatingRecipePanel : MyGUI::IUnlinkWidget
{
    FloatingRecipePanel() : manager(0), root(0), content(0), tabs(0), craftingTab(0),
        minimumWidth(kPanelMinWidth), minimumHeight(0), viewport(0, 0) {}

    void _unlinkWidget(MyGUI::Widget* widget) override
    {
        if (widget == root) root = 0;
        if (widget == content) content = 0;
        if (widget == tabs) tabs = 0;
        if (widget == craftingTab) craftingTab = 0;
    }

    void Disconnect()
    {
        if (manager != 0 && manager == MyGUI::WidgetManager::getInstancePtr())
        {
            manager->unregisterUnlinker(this);
        }
        manager = 0;
        root = 0;
        content = 0;
        tabs = 0;
        craftingTab = 0;
        minimumWidth = kPanelMinWidth;
    }

    MyGUI::WidgetManager* manager;
    MyGUI::Widget* root;
    MyGUI::Widget* content;
    MyGUI::TabControl* tabs;
    MyGUI::TabItem* craftingTab;
    int minimumWidth;
    int minimumHeight;
    MyGUI::IntSize viewport;
};

FloatingRecipePanel g_recipePanel;
bool g_filterPanelCollapsed = false;
bool g_filterPanelResizing = false;
MyGUI::IntSize g_filterPanelDragStartSize(0, 0);
// Keep the player's choices when the current recipe list cannot offer them.
CraftingCategorySelection g_filterPreferences;
bool g_syncingRecipeFilters = false;

struct PendingSearchEditShortcut
{
    PendingSearchEditShortcut()
        : active(false)
        , keyValue(0)
    {
    }

    bool active;
    int keyValue;
    TraderSearchInputBehavior::EditResult editResult;
};

PendingSearchEditShortcut g_pendingSearchEditShortcut;
bool g_haveSearchEditSnapshot = false;
TraderSearchInputBehavior::Snapshot g_searchEditSnapshot;

#define g_showSearchEntryCount (TraderState().core.g_showSearchEntryCount)
#define g_showSearchClearButton (TraderState().core.g_showSearchClearButton)
#define g_autoFocusSearchInput (TraderState().core.g_autoFocusSearchInput)
#define g_searchInputConfiguredWidth (TraderState().core.g_searchInputConfiguredWidth)
#define g_searchInputConfiguredHeight (TraderState().core.g_searchInputConfiguredHeight)

#define g_searchQueryRaw (TraderState().search.g_searchQueryRaw)
#define g_searchQueryNormalized (TraderState().search.g_searchQueryNormalized)
#define g_loggedNumericOnlyQueryIgnored (TraderState().search.g_loggedNumericOnlyQueryIgnored)
#define g_activeTraderTargetId (TraderState().search.g_activeTraderTargetId)
#define g_lastZeroMatchQueryLogged (TraderState().search.g_lastZeroMatchQueryLogged)
#define g_lastSearchSampleQueryLogged (TraderState().search.g_lastSearchSampleQueryLogged)
#define g_lastSearchVisibleEntryCount (TraderState().search.g_lastSearchVisibleEntryCount)
#define g_lastSearchTotalEntryCount (TraderState().search.g_lastSearchTotalEntryCount)

#define g_controlsEnabled (TraderState().core.g_controlsEnabled)

#define g_loggedNoVisibleTraderTarget (TraderState().windowDetection.g_loggedNoVisibleTraderTarget)

#define g_searchFilterDirty (TraderState().search.g_searchFilterDirty)

#define g_prevSearchSlashHotkeyDown (TraderState().searchUi.g_prevSearchSlashHotkeyDown)
#define g_prevToggleHotkeyDown (TraderState().searchUi.g_prevToggleHotkeyDown)
#define g_prevDiagnosticsHotkeyDown (TraderState().searchUi.g_prevDiagnosticsHotkeyDown)
#define g_prevSearchCtrlFHotkeyDown (TraderState().searchUi.g_prevSearchCtrlFHotkeyDown)
#define g_controlsWereInjected (TraderState().searchUi.g_controlsWereInjected)
#define g_suppressNextSearchEditChangeEvent (TraderState().searchUi.g_suppressNextSearchEditChangeEvent)
#define g_pendingSlashFocusBaseQuery (TraderState().search.g_pendingSlashFocusBaseQuery)
#define g_pendingSlashFocusTextSuppression (TraderState().searchUi.g_pendingSlashFocusTextSuppression)
#define g_focusSearchEditOnNextInjection (TraderState().searchUi.g_focusSearchEditOnNextInjection)
#define g_searchContainerDragging (TraderState().searchUi.g_searchContainerDragging)
#define g_searchContainerPositionCustomized (TraderState().searchUi.g_searchContainerPositionCustomized)
#define g_searchContainerPositionOnScreen (TraderState().searchUi.g_searchContainerPositionOnScreen)
#define g_searchContainerDragLastMouseX (TraderState().searchUi.g_searchContainerDragLastMouseX)
#define g_searchContainerDragLastMouseY (TraderState().searchUi.g_searchContainerDragLastMouseY)
#define g_searchContainerDragStartLeft (TraderState().searchUi.g_searchContainerDragStartLeft)
#define g_searchContainerDragStartTop (TraderState().searchUi.g_searchContainerDragStartTop)
#define g_searchContainerStoredLeft (TraderState().searchUi.g_searchContainerStoredLeft)
#define g_searchContainerStoredTop (TraderState().searchUi.g_searchContainerStoredTop)
#define g_searchPanelWidth (TraderState().searchUi.g_searchPanelWidth)
#define g_searchPanelHeight (TraderState().searchUi.g_searchPanelHeight)

#define g_cachedHoveredWidgetInventory (TraderState().binding.g_cachedHoveredWidgetInventory)
#define g_cachedHoveredWidgetInventorySignature (TraderState().binding.g_cachedHoveredWidgetInventorySignature)

bool IsInterestingSearchEditMyGuiKey(MyGUI::KeyCode keyCode)
{
    const int value = keyCode.getValue();
    return value == MyGUI::KeyCode::LeftControl
        || value == MyGUI::KeyCode::RightControl
        || value == MyGUI::KeyCode::ArrowLeft
        || value == MyGUI::KeyCode::ArrowRight
        || value == MyGUI::KeyCode::Backspace
        || value == MyGUI::KeyCode::Tab;
}

void ResetPendingSearchEditShortcut()
{
    g_pendingSearchEditShortcut = PendingSearchEditShortcut();
}

void ResetSearchEditSnapshot()
{
    g_haveSearchEditSnapshot = false;
    g_searchEditSnapshot = TraderSearchInputBehavior::Snapshot();
}

TraderSearchInputBehavior::Text ToSearchInputText(const MyGUI::UString& text)
{
    TraderSearchInputBehavior::Text result;
    const std::size_t length = text.size();
    result.reserve(length);
    for (std::size_t i = 0; i < length; ++i)
    {
        result.push_back(static_cast<TraderSearchInputBehavior::Codepoint>(text[i]));
    }
    return result;
}

MyGUI::UString ToMyGuiText(const TraderSearchInputBehavior::Text& text)
{
    MyGUI::UString result;
    const std::size_t length = text.size();
    for (std::size_t i = 0; i < length; ++i)
    {
        result.push_back(static_cast<MyGUI::UString::unicode_char>(text[i]));
    }
    return result;
}

TraderSearchInputBehavior::Selection CaptureSearchEditSelection(
    MyGUI::EditBox* searchEdit,
    std::size_t textLength)
{
    if (searchEdit == 0 || !searchEdit->isTextSelection())
    {
        return TraderSearchInputBehavior::Selection();
    }

    const std::size_t selectionStart = searchEdit->getTextSelectionStart();
    if (selectionStart == MyGUI::ITEM_NONE)
    {
        return TraderSearchInputBehavior::Selection();
    }

    const std::size_t selectionLength = searchEdit->getTextSelectionLength();
    return TraderSearchInputBehavior::NormalizeSelection(
        TraderSearchInputBehavior::Selection(true, selectionStart, selectionLength),
        textLength);
}

TraderSearchInputBehavior::Snapshot BuildSearchInputSnapshot(
    const MyGUI::UString& text,
    std::size_t cursorPosition,
    const TraderSearchInputBehavior::Selection& selection)
{
    return TraderSearchInputBehavior::Snapshot(
        ToSearchInputText(text),
        cursorPosition,
        TraderSearchInputBehavior::NormalizeSelection(selection, text.size()));
}

TraderSearchInputBehavior::Snapshot CaptureSearchEditSnapshot(MyGUI::EditBox* searchEdit)
{
    if (searchEdit == 0)
    {
        return TraderSearchInputBehavior::Snapshot();
    }

    const MyGUI::UString text = searchEdit->getOnlyText();
    const std::size_t textLength = text.size();
    const std::size_t cursorPosition = TraderSearchInputBehavior::ClampCursor(
        searchEdit->getTextCursor(),
        textLength);
    return BuildSearchInputSnapshot(
        text,
        cursorPosition,
        CaptureSearchEditSelection(searchEdit, textLength));
}

TraderSearchInputBehavior::ShortcutKind ClassifySearchEditShortcut(MyGUI::KeyCode keyCode)
{
    const int keyValue = keyCode.getValue();
    if (keyValue == MyGUI::KeyCode::ArrowLeft)
    {
        return TraderSearchInputBehavior::ShortcutKind_CtrlLeft;
    }

    if (keyValue == MyGUI::KeyCode::ArrowRight)
    {
        return TraderSearchInputBehavior::ShortcutKind_CtrlRight;
    }

    if (keyValue == MyGUI::KeyCode::Backspace)
    {
        return TraderSearchInputBehavior::ShortcutKind_CtrlBackspace;
    }

    return TraderSearchInputBehavior::ShortcutKind_None;
}

void ApplySearchEditSelection(
    MyGUI::EditBox* searchEdit,
    std::size_t cursorPosition,
    const TraderSearchInputBehavior::Selection& selection)
{
    if (searchEdit == 0)
    {
        return;
    }

    const std::size_t textLength = searchEdit->getTextLength();
    const std::size_t clampedCursor = TraderSearchInputBehavior::ClampCursor(cursorPosition, textLength);
    const TraderSearchInputBehavior::Selection normalizedSelection =
        TraderSearchInputBehavior::NormalizeSelection(selection, textLength);
    if (normalizedSelection.active)
    {
        searchEdit->setTextSelection(
            normalizedSelection.start,
            normalizedSelection.start + normalizedSelection.length);
        return;
    }

    searchEdit->setTextCursor(clampedCursor);
    searchEdit->setTextSelection(clampedCursor, clampedCursor);
}

TraderSearchInputBehavior::Snapshot BuildScheduledSearchShortcutSnapshot(
    MyGUI::EditBox* searchEdit,
    TraderSearchInputBehavior::ShortcutKind shortcut)
{
    TraderSearchInputBehavior::Snapshot snapshot = CaptureSearchEditSnapshot(searchEdit);
    if (shortcut == TraderSearchInputBehavior::ShortcutKind_CtrlBackspace && g_haveSearchEditSnapshot)
    {
        snapshot.text = g_searchEditSnapshot.text;
        snapshot.cursor = TraderSearchInputBehavior::ClampCursor(
            g_searchEditSnapshot.cursor,
            snapshot.text.size());
        snapshot.selection = TraderSearchInputBehavior::NormalizeSelection(
            snapshot.selection,
            snapshot.text.size());
    }

    return snapshot;
}

bool ShouldShowAnySearchCountMetric()
{
    return g_showSearchEntryCount;
}

bool SearchHasNoVisibleResults()
{
    return !g_searchQueryNormalized.empty()
        && g_lastSearchTotalEntryCount > 0
        && g_lastSearchVisibleEntryCount == 0;
}

std::string BuildSearchCountCaption(
    std::size_t visibleEntryCount,
    std::size_t totalEntryCount,
    std::size_t visibleQuantity)
{
    (void)visibleQuantity;

    std::stringstream line;
    line << visibleEntryCount << " / " << totalEntryCount;
    return line.str();
}

bool TryGetCurrentMousePosition(int* xOut, int* yOut)
{
    if (xOut == 0 || yOut == 0)
    {
        return false;
    }

    MyGUI::InputManager* inputManager = MyGUI::InputManager::getInstancePtr();
    if (inputManager == 0)
    {
        return false;
    }

    const MyGUI::IntPoint mouse = inputManager->getMousePosition();
    *xOut = mouse.left;
    *yOut = mouse.top;
    return true;
}

MyGUI::IntSize GetRecipePanelViewport()
{
    MyGUI::RenderManager* render = MyGUI::RenderManager::getInstancePtr();
    return render != 0 ? render->getViewSize() : MyGUI::IntSize();
}

int FilterPanelMinimumHeight(int rowCount)
{
    const int controlHeight = kPanelPadding + rowCount * (g_searchInputConfiguredHeight + kPanelGap)
        + kResizeGripSize + kPanelPadding;
    return kPanelHeaderHeight + (std::max)(kPanelMinBodyHeight, controlHeight);
}

MyGUI::IntSize ResolveExpandedPanelSize(const MyGUI::IntSize& view, int minimumWidth, int minimumHeight)
{
    const int automaticWidth = kPanelDefaultWidth + g_searchInputConfiguredWidth - kDefaultSearchInputConfiguredWidth;
    const int preferredWidth = g_searchPanelWidth > 0 ? g_searchPanelWidth : automaticWidth;
    return MyGUI::IntSize(
        (std::min)(view.width, (std::max)(minimumWidth, preferredWidth)),
        (std::min)(view.height, (std::max)(minimumHeight, g_searchPanelHeight)));
}

MyGUI::IntCoord ClampSearchContainerCoord(const MyGUI::IntCoord& inputCoord)
{
    const MyGUI::IntSize view = GetRecipePanelViewport();
    if (view.width <= 0 || view.height <= 0)
    {
        return inputCoord;
    }
    const int maxLeft = (std::max)(0, view.width - inputCoord.width);
    const int maxTop = (std::max)(0, view.height - inputCoord.height);
    const int left = (std::max)(0, (std::min)(inputCoord.left, maxLeft));
    const int top = (std::max)(0, (std::min)(inputCoord.top, maxTop));
    return MyGUI::IntCoord(left, top, inputCoord.width, inputCoord.height);
}

void RememberSearchContainerPosition(MyGUI::Widget* container)
{
    if (container == 0)
    {
        return;
    }

    const MyGUI::IntCoord coord = container->getCoord();
    g_searchContainerStoredLeft = coord.left;
    g_searchContainerStoredTop = coord.top;
    g_searchContainerPositionCustomized = true;
    g_searchContainerPositionOnScreen = true;
}

void RememberSearchEditSnapshotValue(
    const std::string& query,
    std::size_t cursorPosition,
    const TraderSearchInputBehavior::Selection& selection)
{
    g_haveSearchEditSnapshot = true;
    g_searchEditSnapshot = BuildSearchInputSnapshot(MyGUI::UString(query), cursorPosition, selection);
}

void RememberSearchEditSnapshot(MyGUI::EditBox* searchEdit)
{
    if (searchEdit == 0)
    {
        ResetSearchEditSnapshot();
        return;
    }

    g_haveSearchEditSnapshot = true;
    g_searchEditSnapshot = CaptureSearchEditSnapshot(searchEdit);
}

void PersistSearchContainerPosition()
{
    const TraderConfigSnapshot config = CaptureTraderConfigSnapshot();
    SaveTraderConfigSnapshot(config);
}

void LayoutFloatingPanel();

void MoveOrResizeSearchContainer(int deltaX, int deltaY)
{
    if (deltaX == 0 && deltaY == 0)
    {
        return;
    }

    MyGUI::Widget* container = FindControlsContainer();
    if (container == 0)
    {
        return;
    }

    const MyGUI::IntCoord current = container->getCoord();
    if (g_filterPanelResizing)
    {
        if (g_recipePanel.content == 0 || !g_recipePanel.content->getInheritedVisible())
        {
            return;
        }
        const MyGUI::IntSize view = GetRecipePanelViewport();
        if (view.width < kPanelMinWidth || view.height < g_recipePanel.minimumHeight)
        {
            return;
        }
        const int minimumWidth = (std::min)(view.width, g_recipePanel.minimumWidth);
        const int width = (std::max)(minimumWidth, (std::min)(view.width - current.left, current.width + deltaX));
        const int height = (std::max)(g_recipePanel.minimumHeight,
            (std::min)(view.height - current.top, current.height + deltaY));
        if (width != current.width || height != current.height)
        {
            g_searchPanelWidth = width;
            g_searchPanelHeight = height;
            LayoutFloatingPanel();
        }
        return;
    }

    const MyGUI::IntCoord moved = ClampSearchContainerCoord(
        MyGUI::IntCoord(current.left + deltaX, current.top + deltaY, current.width, current.height));
    if (moved.left == current.left && moved.top == current.top)
    {
        return;
    }

    container->setPosition(moved.left, moved.top);
}

void FinalizeSearchContainerDrag(const char* source)
{
    if (!g_searchContainerDragging)
    {
        return;
    }

    g_searchContainerDragging = false;
    const bool wasResize = g_filterPanelResizing;
    g_filterPanelResizing = false;
    MyGUI::Widget* container = FindControlsContainer();
    if (container == 0)
    {
        return;
    }

    std::stringstream line;
    const MyGUI::IntCoord coord = container->getCoord();
    const bool positionChanged =
        coord.left != g_searchContainerDragStartLeft || coord.top != g_searchContainerDragStartTop;
    const bool sizeChanged = wasResize && (coord.width != g_filterPanelDragStartSize.width
        || coord.height != g_filterPanelDragStartSize.height);
    if (positionChanged || sizeChanged)
    {
        RememberSearchContainerPosition(container);
        PersistSearchContainerPosition();
    }

    line << "search container drag finalized"
         << " source=" << (source == 0 ? "<unknown>" : source)
         << " moved=" << (positionChanged ? "true" : "false")
         << " resized=" << (sizeChanged ? "true" : "false")
         << " coord=(" << coord.left << "," << coord.top << "," << coord.width << "," << coord.height << ")";
    LogDebugLine(line.str());
}

void OnSearchDragHandleMousePressed(MyGUI::Widget* sender, int left, int top, MyGUI::MouseButton id)
{
    if (id != MyGUI::MouseButton::Left)
    {
        return;
    }

    MyGUI::Widget* container = FindControlsContainer();
    if (container == 0)
    {
        return;
    }

    const MyGUI::IntCoord coord = container->getCoord();
    g_searchContainerDragStartLeft = coord.left;
    g_searchContainerDragStartTop = coord.top;
    g_filterPanelDragStartSize = MyGUI::IntSize(coord.width, coord.height);
    g_filterPanelResizing = sender->getName() == kCraftingResizeGripName;
    g_searchContainerDragging = true;
    if (!TryGetCurrentMousePosition(&g_searchContainerDragLastMouseX, &g_searchContainerDragLastMouseY))
    {
        g_searchContainerDragLastMouseX = left;
        g_searchContainerDragLastMouseY = top;
    }
}

void OnSearchDragHandleMouseDrag(MyGUI::Widget*, int left, int top, MyGUI::MouseButton id)
{
    if (id != MyGUI::MouseButton::Left || !g_searchContainerDragging)
    {
        return;
    }

    int mouseX = left;
    int mouseY = top;
    TryGetCurrentMousePosition(&mouseX, &mouseY);

    const int deltaX = mouseX - g_searchContainerDragLastMouseX;
    const int deltaY = mouseY - g_searchContainerDragLastMouseY;
    if (deltaX == 0 && deltaY == 0)
    {
        return;
    }

    MoveOrResizeSearchContainer(deltaX, deltaY);
    g_searchContainerDragLastMouseX = mouseX;
    g_searchContainerDragLastMouseY = mouseY;
}

void OnSearchDragHandleMouseMove(MyGUI::Widget*, int left, int top)
{
    if (!g_searchContainerDragging)
    {
        return;
    }

    OnSearchDragHandleMouseDrag(0, left, top, MyGUI::MouseButton::Left);
}

void OnSearchDragHandleMouseReleased(MyGUI::Widget*, int, int, MyGUI::MouseButton id)
{
    if (id != MyGUI::MouseButton::Left)
    {
        return;
    }

    FinalizeSearchContainerDrag("drag_release");
}

void BindPanelDrag(MyGUI::Widget* handle)
{
    handle->setNeedMouseFocus(true);
    handle->setNeedKeyFocus(false);
    handle->eventMouseButtonPressed += MyGUI::newDelegate(&OnSearchDragHandleMousePressed);
    handle->eventMouseMove += MyGUI::newDelegate(&OnSearchDragHandleMouseMove);
    handle->eventMouseDrag += MyGUI::newDelegate(&OnSearchDragHandleMouseDrag);
    handle->eventMouseButtonReleased += MyGUI::newDelegate(&OnSearchDragHandleMouseReleased);
}

bool PlacePanelOutsideWindow(const MyGUI::IntCoord& window, const MyGUI::IntSize& view, MyGUI::IntCoord& panel)
{
    const int right = window.left + window.width + kPanelGap;
    const int left = window.left - panel.width - kPanelGap;
    const int below = window.top + window.height + kPanelGap;
    const int above = window.top - panel.height - kPanelGap;
    if (right + panel.width <= view.width || left >= 0)
    {
        panel.left = right + panel.width <= view.width ? right : left;
        panel.top = window.top;
        return true;
    }
    if (below + panel.height <= view.height || above >= 0)
    {
        panel.left = window.left + window.width - panel.width;
        panel.top = below + panel.height <= view.height ? below : above;
        return true;
    }
    return false;
}

void SetFilterPanelCollapsed(bool collapsed)
{
    if (collapsed && g_recipePanel.root != 0)
    {
        MyGUI::InputManager* input = MyGUI::InputManager::getInstancePtr();
        if (input != 0)
        {
            for (MyGUI::Widget* focus = input->getKeyFocusWidget(); focus != 0; focus = focus->getParent())
            {
                if (focus == g_recipePanel.root)
                {
                    input->resetKeyFocusWidget();
                    break;
                }
            }
        }
    }
    g_filterPanelCollapsed = collapsed;
    LayoutFloatingPanel();
}

void OnCollapseFilterPanel(MyGUI::Widget*)
{
    SetFilterPanelCollapsed(!g_filterPanelCollapsed);
}

void DestroyWidgetDirect(MyGUI::Widget* widget)
{
    if (widget == 0)
    {
        return;
    }

    MyGUI::Gui* gui = MyGUI::Gui::getInstancePtr();
    if (gui != 0)
    {
        gui->destroyWidget(widget);
    }
}

bool IsVirtualKeyDown(int virtualKey)
{
    return virtualKey > 0 && (GetAsyncKeyState(virtualKey) & 0x8000) != 0;
}

bool IsSlashCharacterChordDown(bool shiftDown, bool ctrlDown, bool altDown)
{
    const HKL keyboardLayout = GetKeyboardLayout(0);
    const SHORT slashMapping = VkKeyScanExA('/', keyboardLayout);
    if (slashMapping != -1)
    {
        const int virtualKey = LOBYTE(static_cast<WORD>(slashMapping));
        const BYTE modifierMask = HIBYTE(static_cast<WORD>(slashMapping));
        const bool shiftRequired = (modifierMask & 1U) != 0;
        const bool ctrlRequired = (modifierMask & 2U) != 0;
        const bool altRequired = (modifierMask & 4U) != 0;

        if (shiftDown == shiftRequired
            && ctrlDown == ctrlRequired
            && altDown == altRequired
            && IsVirtualKeyDown(virtualKey))
        {
            return true;
        }
    }

    const bool oemSlashDown = IsVirtualKeyDown(VK_OEM_2);
    const bool numpadSlashDown = IsVirtualKeyDown(VK_DIVIDE);
    return !ctrlDown && !altDown && ((!shiftDown && oemSlashDown) || numpadSlashDown);
}
}

void OnSearchEditKeyPressed(MyGUI::Widget* sender, MyGUI::KeyCode keyCode, MyGUI::Char character);
void OnSearchEditKeyReleased(MyGUI::Widget* sender, MyGUI::KeyCode keyCode);

MyGUI::Widget* FindControlsContainer()
{
    return g_recipePanel.root;
}

MyGUI::EditBox* FindSearchEditBox()
{
    MyGUI::Widget* controlsContainer = FindControlsContainer();
    if (controlsContainer == 0)
    {
        return 0;
    }

    MyGUI::Widget* found = FindNamedDescendantRecursive(controlsContainer, kSearchEditName, false);
    return found == 0 ? 0 : found->castType<MyGUI::EditBox>(false);
}

MyGUI::TextBox* FindSearchPlaceholderTextBox()
{
    MyGUI::Widget* controlsContainer = FindControlsContainer();
    if (controlsContainer == 0)
    {
        return 0;
    }

    MyGUI::Widget* found =
        FindNamedDescendantRecursive(controlsContainer, kSearchPlaceholderName, false);
    return found == 0 ? 0 : found->castType<MyGUI::TextBox>(false);
}

MyGUI::Button* FindSearchClearButton()
{
    MyGUI::Widget* controlsContainer = FindControlsContainer();
    if (controlsContainer == 0)
    {
        return 0;
    }

    MyGUI::Widget* found =
        FindNamedDescendantRecursive(controlsContainer, kSearchClearButtonName, false);
    return found == 0 ? 0 : found->castType<MyGUI::Button>(false);
}

MyGUI::TextBox* FindSearchCountTextBox()
{
    MyGUI::Widget* controlsContainer = FindControlsContainer();
    if (controlsContainer == 0)
    {
        return 0;
    }

    MyGUI::Widget* found = FindNamedDescendantRecursive(controlsContainer, kSearchCountTextName, false);
    return found == 0 ? 0 : found->castType<MyGUI::TextBox>(false);
}

MyGUI::Widget* ResolveTraderParentFromControlsContainer()
{
    return g_recipePanel.craftingTab;
}

void FocusSearchEdit(MyGUI::EditBox* searchEdit, const char* reason)
{
    if (searchEdit == 0)
    {
        return;
    }

    MyGUI::InputManager* input = MyGUI::InputManager::getInstancePtr();
    if (input == 0)
    {
        LogWarnLine("search focus skipped: MyGUI InputManager unavailable");
        return;
    }

    if (g_filterPanelCollapsed)
    {
        SetFilterPanelCollapsed(false);
    }
    if (!searchEdit->getInheritedVisible())
    {
        return;
    }
    input->setKeyFocusWidget(searchEdit);

    std::stringstream line;
    line << "search edit focused"
         << " reason=" << (reason == 0 ? "<unknown>" : reason);
    LogDebugLine(line.str());
}

void BlurSearchEdit(MyGUI::EditBox* searchEdit, const char* reason)
{
    if (searchEdit == 0)
    {
        return;
    }

    MyGUI::InputManager* input = MyGUI::InputManager::getInstancePtr();
    if (input == 0)
    {
        LogWarnLine("search blur skipped: MyGUI InputManager unavailable");
        return;
    }

    if (input->getKeyFocusWidget() != searchEdit)
    {
        return;
    }

    input->setKeyFocusWidget(0);

    std::stringstream line;
    line << "search edit blurred"
         << " reason=" << (reason == 0 ? "<unknown>" : reason);
    LogDebugLine(line.str());
}

void FocusSearchEditIfRequested(MyGUI::EditBox* searchEdit, const char* reason)
{
    if (!g_focusSearchEditOnNextInjection || searchEdit == 0)
    {
        return;
    }

    g_focusSearchEditOnNextInjection = false;
    if (g_autoFocusSearchInput && !g_filterPanelCollapsed)
    {
        FocusSearchEdit(searchEdit, reason);
    }
}

bool IsSearchEditFocused(MyGUI::EditBox* searchEdit)
{
    if (searchEdit == 0)
    {
        return false;
    }

    MyGUI::InputManager* input = MyGUI::InputManager::getInstancePtr();
    if (input == 0)
    {
        return false;
    }

    return input->getKeyFocusWidget() == searchEdit;
}

void UpdateSearchUiState()
{
    MyGUI::EditBox* searchEdit = FindSearchEditBox();
    if (searchEdit == 0)
    {
        return;
    }

    MyGUI::Button* clearButton = FindSearchClearButton();
    MyGUI::TextBox* placeholder = FindSearchPlaceholderTextBox();
    MyGUI::TextBox* countText = FindSearchCountTextBox();

    const bool hasQuery = !g_searchQueryRaw.empty();
    const bool focused = IsSearchEditFocused(searchEdit);
    const bool noVisibleResults = SearchHasNoVisibleResults();

    if (clearButton != 0)
    {
        clearButton->setAlpha(
            hasQuery
                ? (focused ? 1.0f : 0.86f)
                : (focused ? 0.72f : 0.6f));
        clearButton->setColour(
            hasQuery
                ? (noVisibleResults
                    ? MyGUI::Colour(1.0f, 0.78f, 0.78f, 1.0f)
                    : MyGUI::Colour::White)
                : MyGUI::Colour(0.7f, 0.7f, 0.7f, 1.0f));
    }

    if (placeholder != 0)
    {
        placeholder->setVisible(!hasQuery && !focused);
        placeholder->setTextColour(
            focused
                ? MyGUI::Colour(0.82f, 0.82f, 0.82f, 1.0f)
                : MyGUI::Colour(0.63f, 0.63f, 0.63f, 1.0f));
    }

    searchEdit->setAlpha(focused ? 1.0f : 0.92f);
    searchEdit->setColour(
        noVisibleResults
            ? MyGUI::Colour(1.0f, 0.9f, 0.9f, 1.0f)
            : (focused
                ? MyGUI::Colour::White
                : MyGUI::Colour(0.93f, 0.93f, 0.93f, 1.0f)));

    if (countText != 0)
    {
        countText->setTextColour(
            noVisibleResults
                ? MyGUI::Colour(1.0f, 0.42f, 0.42f, 1.0f)
                : MyGUI::Colour(0.83f, 0.83f, 0.83f, 1.0f));
        countText->setAlpha(focused ? 1.0f : 0.92f);
    }
}

namespace
{
void OnCraftingCategoryChanged(MyGUI::ComboBox* combo, std::size_t index)
{
    if (g_syncingRecipeFilters || index >= combo->getItemCount())
    {
        return;
    }

    const CraftingCategoryFilter* option = combo->getItemDataAt<CraftingCategoryFilter>(index, false);
    if (option != 0)
    {
        CraftingCategoryFilter& preference = option->field == CraftingCategoryFilter::ArmorClass
            ? g_filterPreferences.armorClass : g_filterPreferences.category;
        if (!(preference == *option))
        {
            preference = *option;
            MarkSearchFilterDirty("crafting_category_changed");
        }
    }
}

void OnCoveragePartChanged(MyGUI::ComboBox* combo, std::size_t index)
{
    if (!g_syncingRecipeFilters && index < combo->getItemCount())
    {
        const std::string* part = combo->getItemDataAt<std::string>(index, false);
        if (part != 0 && g_filterPreferences.coveragePart != *part)
        {
            g_filterPreferences.coveragePart = *part;
            MarkSearchFilterDirty("crafting_coverage_changed");
        }
    }
}

void OnCraftingSortChanged(MyGUI::ComboBox* combo, std::size_t index)
{
    if (!g_syncingRecipeFilters && index < combo->getItemCount())
    {
        const CraftingRecipeSortMode* mode = combo->getItemDataAt<CraftingRecipeSortMode>(index, false);
        if (mode != 0 && g_filterPreferences.sortMode != *mode)
        {
            g_filterPreferences.sortMode = *mode;
            MarkSearchFilterDirty("crafting_sort_changed");
        }
    }
}

MyGUI::ComboBox* FindRecipeCombo(MyGUI::Widget* parent, const char* name)
{
    MyGUI::Widget* widget = FindNamedDescendantRecursive(parent, name, false);
    return widget != 0 ? widget->castType<MyGUI::ComboBox>(false) : 0;
}

MyGUI::ComboBox* CreateRecipeCombo(
    MyGUI::Widget* parent, const char* name, void (*onChange)(MyGUI::ComboBox*, std::size_t))
{
    const char* skin = "Kenshi_ComboBox";
    MyGUI::ResourceManager* resources = MyGUI::ResourceManager::getInstancePtr();
    if (resources == 0 || !resources->isExist(skin))
    {
        return 0;
    }

    MyGUI::ComboBox* combo = parent->createWidget<MyGUI::ComboBox>(skin, MyGUI::IntCoord(), MyGUI::Align::Default, name);
    if (combo != 0)
    {
        combo->setComboModeDrop(true);
        combo->eventComboChangePosition += MyGUI::newDelegate(onChange);
        // Accept also fires when a player explicitly chooses the current fallback.
        combo->eventComboAccept += MyGUI::newDelegate(onChange);
    }
    return combo;
}

MyGUI::ComboBox* SyncCraftingCategoryCombo(
    MyGUI::Widget* container,
    const char* name,
    const CraftingCategoryOptions& options,
    const CraftingCategoryFilter& preference,
    CraftingCategoryFilter& selection)
{
    MyGUI::ComboBox* combo = FindRecipeCombo(container, name);
    selection = options.front();
    if (options.size() == 1)
    {
        if (combo != 0)
        {
            combo->setVisible(false);
            combo->removeAllItems();
        }
        return 0;
    }

    if (combo == 0)
    {
        combo = CreateRecipeCombo(container, name, &OnCraftingCategoryChanged);
        if (combo == 0)
        {
            return 0;
        }
    }

    bool itemsMatch = combo->getItemCount() == options.size();
    std::size_t selectedIndex = 0;
    for (std::size_t i = 0; i < options.size(); ++i)
    {
        if (itemsMatch)
        {
            const CraftingCategoryFilter* data = combo->getItemDataAt<CraftingCategoryFilter>(i, false);
            itemsMatch = data != 0 && *data == options[i];
        }
        if (options[i] == preference)
        {
            selectedIndex = i;
        }
    }

    if (!itemsMatch)
    {
        combo->removeAllItems();
        for (std::size_t i = 0; i < options.size(); ++i)
        {
            combo->addItem(GetCraftingCategoryLabel(options[i]), MyGUI::Any(options[i]));
        }
    }
    if (!itemsMatch || combo->getIndexSelected() != selectedIndex)
    {
        combo->setIndexSelected(selectedIndex);
    }
    combo->setVisible(true);
    selection = options[selectedIndex];
    return combo;
}

MyGUI::ComboBox* SyncCraftingSortCombo(
    MyGUI::Widget* content, bool canSort, bool hasArmor, bool hasMeleeWeapons, bool hasCoveragePart,
    CraftingRecipeSortMode& selection)
{
    MyGUI::ComboBox* combo = FindRecipeCombo(content, kCraftingSortComboName);
    if (!canSort)
    {
        if (combo != 0)
        {
            combo->setVisible(false);
        }
        return 0;
    }

    if (combo == 0)
    {
        combo = CreateRecipeCombo(content, kCraftingSortComboName, &OnCraftingSortChanged);
        if (combo == 0)
        {
            return 0;
        }
    }

    std::vector<CraftingRecipeSortMode> options;
    std::size_t selectedIndex = 0;
    for (int i = 0; i < CraftingRecipeSortModeCount; ++i)
    {
        const CraftingRecipeSortMode mode = static_cast<CraftingRecipeSortMode>(i);
        const CraftingRecipeSortStat stat = GetCraftingRecipeSortStat(mode);
        const bool supported = stat == CraftingSortStatNone
            || (stat == CraftingSortStatCoverage ? hasCoveragePart
                : (IsCraftingWeaponSortStat(stat) ? hasMeleeWeapons : hasArmor));
        if (!supported)
        {
            continue;
        }
        if (mode == g_filterPreferences.sortMode)
        {
            selectedIndex = options.size();
        }
        options.push_back(mode);
    }
    selection = options[selectedIndex];

    bool itemsMatch = combo->getItemCount() == options.size();
    for (std::size_t i = 0; itemsMatch && i < options.size(); ++i)
    {
        const CraftingRecipeSortMode* mode = combo->getItemDataAt<CraftingRecipeSortMode>(i, false);
        itemsMatch = mode != 0 && *mode == options[i];
    }

    if (!itemsMatch)
    {
        combo->removeAllItems();
        for (std::size_t i = 0; i < options.size(); ++i)
        {
            const CraftingRecipeSortMode mode = options[i];
            combo->addItem(GetCraftingRecipeSortLabel(mode), MyGUI::Any(mode));
        }
    }
    if (combo->getIndexSelected() != selectedIndex)
    {
        combo->setIndexSelected(selectedIndex);
    }
    combo->setVisible(true);
    return combo;
}

std::string SyncCoveragePartCombo(MyGUI::Widget* content, const CraftingCoveragePartOptions& parts)
{
    MyGUI::ComboBox* combo = FindRecipeCombo(content, kCoveragePartComboName);
    if (parts.empty())
    {
        if (combo != 0)
        {
            combo->setVisible(false);
        }
        return std::string();
    }
    if (combo == 0)
    {
        combo = CreateRecipeCombo(content, kCoveragePartComboName, &OnCoveragePartChanged);
        if (combo == 0)
        {
            return std::string();
        }
    }

    typedef std::multimap<std::string, std::string> NamedParts;
    NamedParts namedParts;
    for (CraftingCoveragePartOptions::const_iterator part = parts.begin(); part != parts.end(); ++part)
    {
        namedParts.insert(std::make_pair(part->second, part->first));
    }
    typedef std::pair<std::string, std::string> PartOption;
    std::vector<PartOption> options(1, PartOption("", "Any Body Part"));
    for (NamedParts::const_iterator part = namedParts.begin(); part != namedParts.end(); ++part)
    {
        const std::string label = namedParts.count(part->first) > 1
            ? part->first + " (" + part->second + ")" : part->first;
        options.push_back(PartOption(part->second, label));
    }

    std::size_t selected = 0;
    bool itemsMatch = options.size() == combo->getItemCount();
    for (std::size_t i = 0; i < options.size(); ++i)
    {
        if (g_filterPreferences.coveragePart == options[i].first)
        {
            selected = i;
        }
        if (itemsMatch)
        {
            const std::string* item = combo->getItemDataAt<std::string>(i, false);
            itemsMatch = item != 0 && *item == options[i].first && combo->getItemNameAt(i) == options[i].second;
        }
    }
    if (!itemsMatch)
    {
        combo->removeAllItems();
        for (std::size_t i = 0; i < options.size(); ++i)
        {
            combo->addItem(options[i].second, MyGUI::Any(options[i].first));
        }
    }
    if (combo->getIndexSelected() != selected)
    {
        combo->setIndexSelected(selected);
    }
    combo->setVisible(true);
    return options[selected].first;
}

void OnCoveragePercentChanged(MyGUI::EditBox* edit)
{
    if (g_syncingRecipeFilters)
    {
        return;
    }

    const std::string text = edit->getOnlyText().asUTF8();
    std::string digits;
    for (std::size_t i = 0; i < text.size() && digits.size() < 3; ++i)
    {
        if (text[i] >= '0' && text[i] <= '9')
        {
            digits += text[i];
        }
    }
    if (std::atoi(digits.c_str()) > 100)
    {
        digits = "100";
    }
    if (digits != text)
    {
        edit->setOnlyText(digits);
    }
    const int minimum = std::atoi(digits.c_str());
    if (g_filterPreferences.minimumCoverage != minimum)
    {
        g_filterPreferences.minimumCoverage = minimum;
        MarkSearchFilterDirty("crafting_coverage_changed");
    }
}

int SyncCoverageThreshold(MyGUI::Widget* content, bool hasCoveragePart)
{
    MyGUI::Widget* widget = FindNamedDescendantRecursive(content, kCoveragePercentName, false);
    MyGUI::EditBox* edit = widget != 0 ? widget->castType<MyGUI::EditBox>(false) : 0;
    MyGUI::Widget* label = FindNamedDescendantRecursive(content, kCoveragePercentLabelName, false);
    if (hasCoveragePart && label == 0)
    {
        MyGUI::TextBox* caption = content->createWidget<MyGUI::TextBox>(
            "Kenshi_TextboxStandardText", MyGUI::IntCoord(0, 0, 44, 26), MyGUI::Align::Default, kCoveragePercentLabelName);
        if (caption != 0)
        {
            caption->setCaption("Min %");
            caption->setTextAlign(MyGUI::Align::Left | MyGUI::Align::VCenter);
            caption->setTextColour(MyGUI::Colour(0.83f, 0.83f, 0.83f, 1.0f));
            caption->setNeedMouseFocus(false);
            label = caption;
        }
    }
    if (hasCoveragePart && label != 0 && edit == 0)
    {
        edit = content->createWidget<MyGUI::EditBox>(
            "Kenshi_EditBox", MyGUI::IntCoord(0, 0, 42, 26), MyGUI::Align::Default, kCoveragePercentName);
        if (edit != 0)
        {
            edit->setMaxTextLength(3);
            std::ostringstream minimum;
            minimum << g_filterPreferences.minimumCoverage;
            edit->setOnlyText(minimum.str());
            edit->eventEditTextChange += MyGUI::newDelegate(&OnCoveragePercentChanged);
        }
    }
    if (label != 0)
    {
        label->setVisible(hasCoveragePart && edit != 0);
    }
    if (edit != 0)
    {
        edit->setVisible(hasCoveragePart);
        return hasCoveragePart ? g_filterPreferences.minimumCoverage : 0;
    }
    return 0;
}

void LayoutSearchRow(int width, int rowHeight)
{
    MyGUI::TextBox* count = FindSearchCountTextBox();
    MyGUI::Button* clear = FindSearchClearButton();
    MyGUI::EditBox* edit = FindSearchEditBox();
    MyGUI::TextBox* placeholder = FindSearchPlaceholderTextBox();
    int right = width - kPanelPadding;

    const bool showCount = ShouldShowAnySearchCountMetric();
    count->setVisible(showCount);
    if (showCount)
    {
        count->setCoord(right - kSearchCountWidth, kPanelPadding, kSearchCountWidth, rowHeight);
        right -= kSearchCountWidth + kPanelGap;
    }

    clear->setVisible(g_showSearchClearButton);
    if (g_showSearchClearButton)
    {
        const int buttonSize = (std::min)(rowHeight, 32);
        clear->setCoord(right - buttonSize, kPanelPadding + (rowHeight - buttonSize) / 2, buttonSize, buttonSize);
        right -= buttonSize + kPanelGap;
    }
    edit->setCoord(kPanelPadding, kPanelPadding, right - kPanelPadding, rowHeight);
    placeholder->setCoord(kPanelPadding + 10, kPanelPadding, edit->getWidth() - 16, rowHeight);
}

void SetFilterLabel(MyGUI::Widget* content, const char* name, const char* text, bool show)
{
    MyGUI::Widget* widget = FindNamedDescendantRecursive(content, name, false);
    MyGUI::TextBox* label = widget != 0 ? widget->castType<MyGUI::TextBox>(false) : 0;
    if (label == 0 && show)
    {
        label = content->createWidget<MyGUI::TextBox>("Kenshi_TextboxStandardText",
            MyGUI::IntCoord(0, 0, kFilterLabelWidth, 26), MyGUI::Align::Default, name);
        if (label != 0)
        {
            label->setNeedMouseFocus(false);
            label->setTextAlign(MyGUI::Align::Left | MyGUI::Align::VCenter);
            label->setTextColour(MyGUI::Colour(0.83f, 0.83f, 0.83f, 1.0f));
        }
    }
    if (label != 0)
    {
        label->setVisible(show);
        label->setCaption(text);
    }
}

int MeasureRecipeFilterWidth(MyGUI::Widget* content, MyGUI::ComboBox* const* combos, std::size_t count)
{
    MyGUI::Widget* widget = FindNamedDescendantRecursive(content, kComboTextMeasureName, false);
    MyGUI::TextBox* measure = widget != 0 ? widget->castType<MyGUI::TextBox>(false) : 0;
    if (measure == 0)
    {
        measure = content->createWidget<MyGUI::TextBox>("Kenshi_TextboxStandardText",
            MyGUI::IntCoord(0, 0, 1024, 26), MyGUI::Align::Default, kComboTextMeasureName);
        if (measure == 0)
        {
            return kPanelMinWidth;
        }
        measure->setVisible(false);
        measure->setNeedMouseFocus(false);
    }
    int textWidth = 0;
    for (std::size_t i = 0; i < count; ++i)
    {
        MyGUI::ComboBox* combo = combos[i];
        if (combo == 0)
        {
            continue;
        }
        for (std::size_t item = 0; item < combo->getItemCount(); ++item)
        {
            measure->setCaption(combo->getItemNameAt(item));
            textWidth = (std::max)(textWidth, measure->getTextSize().width);
        }
    }
    return (std::max)(kPanelMinWidth, 2 * kPanelPadding + kFilterLabelWidth + kPanelGap + textWidth + kComboTextPadding);
}

void LayoutFilterRow(MyGUI::Widget* content, MyGUI::ComboBox* combo, const char* labelName,
    int width, int height, int& top)
{
    if (combo == 0 || !combo->getVisible())
    {
        return;
    }
    MyGUI::Widget* label = FindNamedDescendantRecursive(content, labelName, false);
    const int labelSpace = label != 0 ? kFilterLabelWidth + kPanelGap : 0;
    if (label != 0)
    {
        label->setCoord(kPanelPadding, top, kFilterLabelWidth, height);
    }
    combo->setCoord(kPanelPadding + labelSpace, top, width - labelSpace, height);
    top += height + kPanelGap;
}

void LayoutFloatingPanel()
{
    MyGUI::Widget* root = g_recipePanel.root;
    MyGUI::Widget* content = g_recipePanel.content;
    if (root == 0 || content == 0)
    {
        return;
    }

    MyGUI::ComboBox* category = FindRecipeCombo(content, kCraftingCategoryComboName);
    MyGUI::ComboBox* armorClass = FindRecipeCombo(content, kCraftingArmorClassComboName);
    MyGUI::ComboBox* coveragePart = FindRecipeCombo(content, kCoveragePartComboName);
    MyGUI::ComboBox* sort = FindRecipeCombo(content, kCraftingSortComboName);
    const bool showCategory = category != 0 && category->getVisible();
    const bool showArmorClass = armorClass != 0 && armorClass->getVisible();
    const bool showCoverage = coveragePart != 0 && coveragePart->getVisible();
    const bool showSort = sort != 0 && sort->getVisible();
    MyGUI::Widget* percent = FindNamedDescendantRecursive(content, kCoveragePercentName, false);
    MyGUI::Widget* percentLabel = FindNamedDescendantRecursive(content, kCoveragePercentLabelName, false);
    const bool showThreshold = showCoverage && percent != 0 && percentLabel != 0 && percent->getVisible();

    const MyGUI::IntSize view = GetRecipePanelViewport();
    if (view.width < kCollapsedPanelWidth || view.height < kPanelHeaderHeight)
    {
        return;
    }
    g_recipePanel.viewport = view;
    g_recipePanel.minimumHeight = FilterPanelMinimumHeight(
        1 + showCategory + showArmorClass + showCoverage + showThreshold + showSort);
    const MyGUI::IntSize expandedSize = ResolveExpandedPanelSize(view, g_recipePanel.minimumWidth, g_recipePanel.minimumHeight);
    const bool collapsed = g_filterPanelCollapsed || view.width < kPanelMinWidth
        || view.height < g_recipePanel.minimumHeight;
    MyGUI::IntCoord coord = root->getCoord();
    coord.width = collapsed ? (std::min)(kCollapsedPanelWidth, view.width) : expandedSize.width;
    coord.height = collapsed ? kPanelHeaderHeight : expandedSize.height;
    root->setCoord(ClampSearchContainerCoord(coord));

    MyGUI::Widget* header = FindNamedDescendantRecursive(root, kCraftingHeaderName, false);
    MyGUI::Widget* handle = FindNamedDescendantRecursive(header, kSearchDragHandleName, false);
    MyGUI::Widget* collapse = FindNamedDescendantRecursive(header, kCraftingCollapseButtonName, false);
    const int collapseLeft = coord.width - kPanelPadding - 26;
    header->setSize(coord.width, kPanelHeaderHeight);
    handle->setCoord(kPanelPadding, 0, collapseLeft - kPanelPadding - kPanelGap, kPanelHeaderHeight);
    collapse->setPosition(collapseLeft, 6);
    collapse->castType<MyGUI::Button>()->setCaption(collapsed ? "+" : "-");

    content->setVisible(!collapsed);
    if (collapsed)
    {
        return;
    }

    content->setSize(coord.width, coord.height - kPanelHeaderHeight);
    const int rowHeight = g_searchInputConfiguredHeight;
    LayoutSearchRow(content->getWidth(), rowHeight);

    const int rowWidth = content->getWidth() - 2 * kPanelPadding;
    int rowTop = kPanelPadding + rowHeight + kPanelGap;
    LayoutFilterRow(content, category, kCraftingCategoryLabelName, rowWidth, rowHeight, rowTop);
    LayoutFilterRow(content, armorClass, kCraftingArmorClassLabelName, rowWidth, rowHeight, rowTop);
    LayoutFilterRow(content, coveragePart, kCoveragePartLabelName, rowWidth, rowHeight, rowTop);
    if (showThreshold)
    {
        percentLabel->setCoord(kPanelPadding, rowTop, kFilterLabelWidth, rowHeight);
        percent->setCoord(kPanelPadding + kFilterLabelWidth + kPanelGap, rowTop, 42, rowHeight);
        rowTop += rowHeight + kPanelGap;
    }
    LayoutFilterRow(content, sort, kCraftingSortLabelName, rowWidth, rowHeight, rowTop);

    MyGUI::Widget* grip = FindNamedDescendantRecursive(content, kCraftingResizeGripName, false);
    grip->setCoord(content->getWidth() - kResizeGripSize - 3, content->getHeight() - kResizeGripSize - 3,
        kResizeGripSize, kResizeGripSize);
}
}

CraftingCategorySelection UpdateCraftingCategoryControls(
    const CraftingRecipeCategories& recipes, bool canSort, const CraftingCoveragePartOptions& parts)
{
    CraftingCategorySelection selection;
    MyGUI::Widget* container = g_recipePanel.content;
    MyGUI::EditBox* searchEdit = FindSearchEditBox();
    if (container == 0 || searchEdit == 0)
    {
        return selection;
    }

    g_syncingRecipeFilters = true;
    const CraftingCategoryOptions categories = GetCraftingCategoryOptions(recipes);
    MyGUI::ComboBox* category = SyncCraftingCategoryCombo(
        container, kCraftingCategoryComboName, categories, g_filterPreferences.category, selection.category);
    const CraftingCategoryOptions armorClasses = GetCraftingArmorClassOptions(
        category != 0 ? recipes : CraftingRecipeCategories(), selection.category);
    MyGUI::ComboBox* armorClass = SyncCraftingCategoryCombo(
        container, kCraftingArmorClassComboName, armorClasses, g_filterPreferences.armorClass, selection.armorClass);
    bool hasArmor = false;
    bool hasWeapons = false;
    bool hasMeleeWeapons = false;
    for (CraftingRecipeCategories::const_iterator recipe = recipes.begin(); recipe != recipes.end(); ++recipe)
    {
        hasArmor = hasArmor || recipe->kind == CraftingRecipeCategory::Armor;
        hasWeapons = hasWeapons || recipe->kind == CraftingRecipeCategory::Weapon;
        hasMeleeWeapons = hasMeleeWeapons || recipe->supportsWeaponStats;
    }
    selection.coveragePart = SyncCoveragePartCombo(container, parts);
    const bool hasCoveragePart = !selection.coveragePart.empty();
    selection.minimumCoverage = SyncCoverageThreshold(container, hasCoveragePart);
    MyGUI::ComboBox* sort = SyncCraftingSortCombo(
        container, canSort, hasArmor, hasMeleeWeapons, hasCoveragePart, selection.sortMode);
    MyGUI::ComboBox* coverage = parts.empty() ? 0 : FindRecipeCombo(container, kCoveragePartComboName);
    const char* categoryLabel = hasArmor && hasWeapons ? "Category" : (hasArmor ? "Slot" : "Weapon");
    SetFilterLabel(container, kCraftingCategoryLabelName, categoryLabel, category != 0);
    SetFilterLabel(container, kCraftingArmorClassLabelName, "Class", armorClass != 0);
    SetFilterLabel(container, kCoveragePartLabelName, "Coverage", coverage != 0);
    SetFilterLabel(container, kCraftingSortLabelName, "Sort", sort != 0);
    MyGUI::ComboBox* combos[] = { category, armorClass, coverage, sort };
    g_recipePanel.minimumWidth = MeasureRecipeFilterWidth(container, combos, sizeof(combos) / sizeof(combos[0]));
    LayoutFloatingPanel();
    g_syncingRecipeFilters = false;
    return selection;
}

void UpdateSearchCountText(
    std::size_t visibleEntryCount,
    std::size_t totalEntryCount,
    std::size_t visibleQuantity)
{
    g_lastSearchVisibleEntryCount = visibleEntryCount;
    g_lastSearchTotalEntryCount = totalEntryCount;
    MyGUI::TextBox* countText = FindSearchCountTextBox();
    if (countText == 0)
    {
        UpdateSearchUiState();
        return;
    }

    const std::string caption =
        BuildSearchCountCaption(visibleEntryCount, totalEntryCount, visibleQuantity);
    countText->setCaption(caption);
    UpdateSearchUiState();
}

void TickSearchContainerDrag()
{
    if (!g_searchContainerDragging)
    {
        return;
    }

    int mouseX = 0;
    int mouseY = 0;
    if (TryGetCurrentMousePosition(&mouseX, &mouseY))
    {
        const int deltaX = mouseX - g_searchContainerDragLastMouseX;
        const int deltaY = mouseY - g_searchContainerDragLastMouseY;
        MoveOrResizeSearchContainer(deltaX, deltaY);
        g_searchContainerDragLastMouseX = mouseX;
        g_searchContainerDragLastMouseY = mouseY;
    }

    if ((GetAsyncKeyState(VK_LBUTTON) & 0x8000) == 0)
    {
        FinalizeSearchContainerDrag("drag_release_poll");
    }
}

static bool BuildControlsScaffold(
    MyGUI::TabControl* tabs,
    MyGUI::TabItem* craftingTab,
    const SearchUiCallbacks& callbacks)
{
    MyGUI::Gui* gui = MyGUI::Gui::getInstancePtr();
    MyGUI::WidgetManager* widgets = MyGUI::WidgetManager::getInstancePtr();
    MyGUI::LayerManager* layers = MyGUI::LayerManager::getInstancePtr();
    const MyGUI::IntSize view = GetRecipePanelViewport();
    const char* layer = "Window";
    if (tabs == 0 || craftingTab == 0 || gui == 0 || widgets == 0 || layers == 0 || !layers->isExist(layer)
        || view.width < kCollapsedPanelWidth || view.height < kPanelHeaderHeight)
    {
        return false;
    }
    if (callbacks.onSearchTextChanged == 0
        || callbacks.onSearchEditFocusChanged == 0
        || callbacks.onSearchPlaceholderClicked == 0
        || callbacks.onSearchClearButtonClicked == 0)
    {
        LogErrorLine("failed to build controls scaffold: missing search UI callbacks");
        return false;
    }

    const bool recipeTab = FindNamedDescendantByTokenRecursive(craftingTab, kCraftingRecipesPanelName, true) != 0;
    const MyGUI::IntSize expandedSize = ResolveExpandedPanelSize(
        view, kPanelMinWidth, FilterPanelMinimumHeight(recipeTab ? 5 : 1));
    MyGUI::IntCoord coord(kPanelPadding, kPanelPadding, expandedSize.width, expandedSize.height);
    if (g_searchContainerPositionCustomized && g_searchContainerPositionOnScreen)
    {
        coord.left = g_searchContainerStoredLeft;
        coord.top = g_searchContainerStoredTop;
    }
    else
    {
        const MyGUI::Window* window = FindOwningWindow(craftingTab);
        const MyGUI::IntCoord bounds = window != 0 ? window->getAbsoluteCoord() : craftingTab->getAbsoluteCoord();
        if (g_filterPanelCollapsed || !PlacePanelOutsideWindow(bounds, view, coord))
        {
            g_filterPanelCollapsed = true;
            coord.width = kCollapsedPanelWidth;
            coord.height = kPanelHeaderHeight;
            PlacePanelOutsideWindow(bounds, view, coord);
        }
    }

    MyGUI::Widget* root = gui->createWidget<MyGUI::Widget>(
        "PanelEmpty",
        ClampSearchContainerCoord(coord),
        MyGUI::Align::Default,
        layer,
        kControlsContainerName);
    if (root == 0)
    {
        return false;
    }
    root->setNeedMouseFocus(false);
    root->setInheritsPick(true);

    MyGUI::Widget* container = root->createWidget<MyGUI::Widget>(
        "Kenshi_GenericWindowSkin", MyGUI::IntCoord(0, kPanelHeaderHeight, expandedSize.width, kPanelMinBodyHeight),
        MyGUI::Align::Default, kCraftingFilterContentName);
    MyGUI::Widget* header = root->createWidget<MyGUI::Widget>(
        "Kenshi_GenericWindowHeaderSkin", MyGUI::IntCoord(0, 0, coord.width, kPanelHeaderHeight),
        MyGUI::Align::Default, kCraftingHeaderName);
    if (container == 0 || header == 0)
    {
        DestroyWidgetDirect(root);
        return false;
    }
    container->setNeedMouseFocus(false);
    container->setInheritsPick(true);
    header->setNeedMouseFocus(false);
    header->setInheritsPick(true);

    MyGUI::TextBox* dragHandle = header->createWidget<MyGUI::TextBox>(
        "Kenshi_TextboxPaintedText", MyGUI::IntCoord(kPanelPadding, 0, 110, kPanelHeaderHeight),
        MyGUI::Align::Default, kSearchDragHandleName);
    MyGUI::Button* collapse = header->createWidget<MyGUI::Button>(
        "Kenshi_Button1", MyGUI::IntCoord(0, 0, 26, 26),
        MyGUI::Align::Default, kCraftingCollapseButtonName);
    MyGUI::TextBox* grip = container->createWidget<MyGUI::TextBox>(
        "Kenshi_TextboxStandardText", MyGUI::IntCoord(0, 0, kResizeGripSize, kResizeGripSize),
        MyGUI::Align::Default, kCraftingResizeGripName);
    if (dragHandle == 0 || collapse == 0 || grip == 0)
    {
        DestroyWidgetDirect(root);
        return false;
    }
    dragHandle->setCaption("Crafting filters");
    dragHandle->setTextAlign(MyGUI::Align::HCenter | MyGUI::Align::VCenter);
    BindPanelDrag(dragHandle);
    collapse->eventMouseButtonClick += MyGUI::newDelegate(&OnCollapseFilterPanel);
    grip->setCaption("//");
    grip->setTextAlign(MyGUI::Align::HCenter | MyGUI::Align::VCenter);
    grip->setTextColour(MyGUI::Colour(0.62f, 0.62f, 0.62f, 1.0f));
    BindPanelDrag(grip);

    MyGUI::EditBox* searchEdit = container->createWidget<MyGUI::EditBox>(
        "Kenshi_EditBox", MyGUI::IntCoord(0, 0, 140, 26), MyGUI::Align::Default, kSearchEditName);
    MyGUI::TextBox* placeholder = container->createWidget<MyGUI::TextBox>(
        "Kenshi_TextboxStandardText", MyGUI::IntCoord(0, 0, 140, 26), MyGUI::Align::Default, kSearchPlaceholderName);
    MyGUI::Button* clearButton = container->createWidget<MyGUI::Button>(
        "Kenshi_Button1", MyGUI::IntCoord(0, 0, 26, 26), MyGUI::Align::Default, kSearchClearButtonName);
    MyGUI::TextBox* countText = container->createWidget<MyGUI::TextBox>(
        "Kenshi_TextboxStandardText", MyGUI::IntCoord(0, 0, kSearchCountWidth, 26), MyGUI::Align::Default, kSearchCountTextName);
    if (searchEdit == 0 || placeholder == 0 || clearButton == 0 || countText == 0)
    {
        LogErrorLine("failed to create floating panel search controls");
        DestroyWidgetDirect(root);
        return false;
    }

    searchEdit->setOnlyText(g_searchQueryRaw);
    searchEdit->eventEditTextChange += MyGUI::newDelegate(callbacks.onSearchTextChanged);
    searchEdit->eventKeySetFocus += MyGUI::newDelegate(callbacks.onSearchEditFocusChanged);
    searchEdit->eventKeyLostFocus += MyGUI::newDelegate(callbacks.onSearchEditFocusChanged);
    searchEdit->eventKeyButtonPressed += MyGUI::newDelegate(&OnSearchEditKeyPressed);
    searchEdit->eventKeyButtonReleased += MyGUI::newDelegate(&OnSearchEditKeyReleased);
    placeholder->setCaption("Search items...");
    placeholder->setTextAlign(MyGUI::Align::Left | MyGUI::Align::VCenter);
    placeholder->setNeedMouseFocus(true);
    placeholder->eventMouseButtonClick += MyGUI::newDelegate(callbacks.onSearchPlaceholderClicked);
    clearButton->setCaption("x");
    clearButton->eventMouseButtonClick += MyGUI::newDelegate(callbacks.onSearchClearButtonClicked);
    countText->setTextAlign(MyGUI::Align::Right | MyGUI::Align::VCenter);
    countText->setNeedMouseFocus(false);

    widgets->registerUnlinker(&g_recipePanel);
    g_recipePanel.manager = widgets;
    g_recipePanel.root = root;
    g_recipePanel.content = container;
    g_recipePanel.tabs = tabs;
    g_recipePanel.craftingTab = craftingTab;
    LayoutFloatingPanel();
    UpdateSearchCountText(0, 0, 0);
    FocusSearchEditIfRequested(searchEdit, "controls_built");
    RememberSearchEditSnapshot(searchEdit);

    return true;
}

SearchFocusHotkeyKind DetectSearchFocusHotkeyPressedEdge(MyGUI::EditBox* searchEdit)
{
    if (searchEdit == 0 || key == 0 || key->keyboard == 0)
    {
        g_prevSearchSlashHotkeyDown = false;
        g_prevSearchCtrlFHotkeyDown = false;
        return SearchFocusHotkeyKind_None;
    }

    const bool searchFocused = IsSearchEditFocused(searchEdit);
    const bool ctrlDown = key->keyboard->isKeyDown(OIS::KC_LCONTROL)
        || key->keyboard->isKeyDown(OIS::KC_RCONTROL);
    const bool altDown = key->keyboard->isKeyDown(OIS::KC_LMENU)
        || key->keyboard->isKeyDown(OIS::KC_RMENU);
    const bool shiftDown = key->keyboard->isKeyDown(OIS::KC_LSHIFT)
        || key->keyboard->isKeyDown(OIS::KC_RSHIFT);
    const bool fDown = key->keyboard->isKeyDown(OIS::KC_F);

    const bool slashChordDown =
        !searchFocused && IsSlashCharacterChordDown(shiftDown, ctrlDown, altDown);
    const bool ctrlFChordDown = !searchFocused && ctrlDown && fDown;

    const bool slashPressedEdge = slashChordDown && !g_prevSearchSlashHotkeyDown;
    const bool ctrlFPressedEdge = ctrlFChordDown && !g_prevSearchCtrlFHotkeyDown;

    g_prevSearchSlashHotkeyDown = slashChordDown;
    g_prevSearchCtrlFHotkeyDown = ctrlFChordDown;

    if (slashPressedEdge)
    {
        return SearchFocusHotkeyKind_Slash;
    }
    if (ctrlFPressedEdge)
    {
        return SearchFocusHotkeyKind_CtrlF;
    }

    return SearchFocusHotkeyKind_None;
}

bool TryStripSingleSlashShortcutInsertion(
    const std::string& currentText,
    const std::string& baseText,
    std::string* outRestoredText)
{
    if (outRestoredText == 0 || currentText.size() != baseText.size() + 1)
    {
        return false;
    }

    for (std::size_t index = 0; index < currentText.size(); ++index)
    {
        if (currentText[index] != '/')
        {
            continue;
        }

        const std::string restored =
            currentText.substr(0, index) + currentText.substr(index + 1);
        if (restored == baseText)
        {
            *outRestoredText = restored;
            return true;
        }
    }

    return false;
}

void SetSearchQueryAndRefresh(
    MyGUI::EditBox* searchEdit,
    const std::string& rawText,
    const char* reason,
    bool focusAfterSet)
{
    if (searchEdit == 0)
    {
        return;
    }

    g_searchQueryRaw = rawText;
    g_searchQueryNormalized = NormalizeSearchText(g_searchQueryRaw);
    g_loggedNumericOnlyQueryIgnored = false;
    g_lastSearchSampleQueryLogged.clear();
    g_lastZeroMatchQueryLogged.clear();
    g_pendingSlashFocusBaseQuery.clear();
    g_pendingSlashFocusTextSuppression = false;

    const std::string currentOnlyText = searchEdit->getOnlyText().asUTF8();
    if (currentOnlyText != rawText)
    {
        g_suppressNextSearchEditChangeEvent = true;
        searchEdit->setOnlyText(rawText);
    }

    if (focusAfterSet)
    {
        FocusSearchEdit(searchEdit, reason);
    }

    std::stringstream line;
    line << "search ui action"
         << " reason=" << (reason == 0 ? "<unknown>" : reason)
         << " raw=\"" << TruncateForLog(g_searchQueryRaw, 64) << "\""
         << " normalized=\"" << TruncateForLog(g_searchQueryNormalized, 64) << "\"";
    LogSearchDebugLine(line.str());

    MarkSearchFilterDirty(reason);
    ApplySearchFilterFromControls(false, ShouldLogSearchDebug());
    UpdateSearchUiState();
    RememberSearchEditSnapshot(searchEdit);
}

void ApplySearchShortcutQueryAndCursor(
    MyGUI::EditBox* searchEdit,
    const TraderSearchInputBehavior::EditResult& editResult,
    const char* reason)
{
    if (searchEdit == 0 || !editResult.handled || !editResult.rewriteText)
    {
        return;
    }

    const std::string rawText = ToMyGuiText(editResult.text).asUTF8();
    SetSearchQueryAndRefresh(searchEdit, rawText, reason, false);
    ApplySearchEditSelection(searchEdit, editResult.cursor, editResult.selection);
    RememberSearchEditSnapshotValue(rawText, editResult.cursor, editResult.selection);
}

bool ScheduleSearchEditMyGuiShortcut(MyGUI::EditBox* searchEdit, MyGUI::KeyCode keyCode)
{
    if (searchEdit == 0)
    {
        return false;
    }

    MyGUI::InputManager* inputManager = MyGUI::InputManager::getInstancePtr();
    if (inputManager == 0 || !inputManager->isControlPressed())
    {
        return false;
    }

    const TraderSearchInputBehavior::ShortcutKind shortcut = ClassifySearchEditShortcut(keyCode);
    if (shortcut == TraderSearchInputBehavior::ShortcutKind_None)
    {
        return false;
    }

    ResetPendingSearchEditShortcut();
    g_pendingSearchEditShortcut.active = true;
    g_pendingSearchEditShortcut.keyValue = keyCode.getValue();
    g_pendingSearchEditShortcut.editResult = TraderSearchInputBehavior::ApplyShortcut(
        shortcut,
        BuildScheduledSearchShortcutSnapshot(searchEdit, shortcut));
    if (!g_pendingSearchEditShortcut.editResult.handled)
    {
        ResetPendingSearchEditShortcut();
        return false;
    }

    return true;
}

void ApplyPendingSearchEditShortcut(MyGUI::EditBox* searchEdit, MyGUI::KeyCode keyCode)
{
    if (!g_pendingSearchEditShortcut.active || g_pendingSearchEditShortcut.keyValue != keyCode.getValue())
    {
        return;
    }

    const PendingSearchEditShortcut pending = g_pendingSearchEditShortcut;
    ResetPendingSearchEditShortcut();

    if (searchEdit == 0)
    {
        return;
    }

    if (!pending.editResult.handled)
    {
        return;
    }

    if (pending.editResult.rewriteText)
    {
        ApplySearchShortcutQueryAndCursor(
            searchEdit,
            pending.editResult,
            "ctrl_shortcut");
        return;
    }

    ApplySearchEditSelection(searchEdit, pending.editResult.cursor, pending.editResult.selection);
    RememberSearchEditSnapshot(searchEdit);
}

void OnSearchClearButtonClicked(MyGUI::Widget*)
{
    MyGUI::EditBox* searchEdit = FindSearchEditBox();
    if (searchEdit == 0)
    {
        return;
    }

    if (g_searchQueryRaw.empty())
    {
        FocusSearchEdit(searchEdit, "clear_button_empty");
        return;
    }

    SetSearchQueryAndRefresh(searchEdit, "", "clear_button", true);
}

void OnSearchPlaceholderClicked(MyGUI::Widget*)
{
    MyGUI::EditBox* searchEdit = FindSearchEditBox();
    if (searchEdit == 0)
    {
        return;
    }

    FocusSearchEdit(searchEdit, "placeholder_click");
}

void OnSearchEditKeyFocusChanged(MyGUI::Widget*, MyGUI::Widget*)
{
    ResetPendingSearchEditShortcut();
    UpdateSearchUiState();
}

void OnSearchEditKeyPressed(MyGUI::Widget* sender, MyGUI::KeyCode keyCode, MyGUI::Char character)
{
    (void)character;

    if (sender == 0 || !IsInterestingSearchEditMyGuiKey(keyCode))
    {
        return;
    }

    MyGUI::EditBox* searchEdit = sender->castType<MyGUI::EditBox>(false);
    if (searchEdit == 0)
    {
        return;
    }

    if (keyCode.getValue() == MyGUI::KeyCode::Tab)
    {
        BlurSearchEdit(searchEdit, "tab_key");
        ResetPendingSearchEditShortcut();
        RememberSearchEditSnapshot(searchEdit);
        UpdateSearchUiState();
        return;
    }

    ScheduleSearchEditMyGuiShortcut(searchEdit, keyCode);
}

void OnSearchEditKeyReleased(MyGUI::Widget* sender, MyGUI::KeyCode keyCode)
{
    if (sender == 0)
    {
        ResetPendingSearchEditShortcut();
        ResetSearchEditSnapshot();
        return;
    }

    MyGUI::EditBox* searchEdit = sender->castType<MyGUI::EditBox>(false);
    if (searchEdit == 0)
    {
        ResetPendingSearchEditShortcut();
        return;
    }

    if (IsInterestingSearchEditMyGuiKey(keyCode))
    {
        ApplyPendingSearchEditShortcut(searchEdit, keyCode);
    }

    RememberSearchEditSnapshot(searchEdit);
}

void OnSearchTextChanged(MyGUI::EditBox* sender)
{
    if (sender == 0)
    {
        return;
    }

    const std::string captionText = sender->getCaption().asUTF8();
    const std::string onlyText = sender->getOnlyText().asUTF8();
    if (g_suppressNextSearchEditChangeEvent && onlyText == g_searchQueryRaw)
    {
        g_suppressNextSearchEditChangeEvent = false;
        return;
    }
    g_suppressNextSearchEditChangeEvent = false;

    if (g_pendingSlashFocusTextSuppression)
    {
        std::string restoredText;
        if (TryStripSingleSlashShortcutInsertion(
                onlyText,
                g_pendingSlashFocusBaseQuery,
                &restoredText))
        {
            g_pendingSlashFocusTextSuppression = false;
            g_suppressNextSearchEditChangeEvent = true;
            sender->setOnlyText(restoredText);
            return;
        }

        g_pendingSlashFocusTextSuppression = false;
    }

    g_searchQueryRaw = onlyText;
    g_searchQueryNormalized = NormalizeSearchText(g_searchQueryRaw);
    g_loggedNumericOnlyQueryIgnored = false;

    std::stringstream line;
    line << "search input changed"
         << " raw=\"" << TruncateForLog(g_searchQueryRaw, 64) << "\""
         << " normalized=\"" << TruncateForLog(g_searchQueryNormalized, 64) << "\""
         << " raw_len=" << g_searchQueryRaw.size()
         << " caption_len=" << captionText.size()
         << " only_len=" << onlyText.size();
    LogSearchDebugLine(line.str());

    MarkSearchFilterDirty("text_changed");
    ApplySearchFilterFromControls(false, ShouldLogSearchDebug());
    UpdateSearchUiState();
    RememberSearchEditSnapshot(sender);
}

void DestroyControlsIfPresent()
{
    MyGUI::Widget* controlsContainer = FindControlsContainer();
    FinalizeSearchContainerDrag("controls_closed");
    g_recipePanel.Disconnect();
    if (controlsContainer != 0)
    {
        DestroyWidgetDirect(controlsContainer);
        LogDebugLine("controls container destroyed");
    }
    g_searchContainerDragging = false;
    g_searchContainerDragStartLeft = 0;
    g_searchContainerDragStartTop = 0;

    g_controlsWereInjected = false;
    ResetPendingSearchEditShortcut();
    ResetSearchEditSnapshot();
    g_searchFilterDirty = false;
    ResetObservedTraderEntriesState();
    g_pendingSlashFocusBaseQuery.clear();
    g_pendingSlashFocusTextSuppression = false;
    g_suppressNextSearchEditChangeEvent = false;
    g_cachedHoveredWidgetInventory = 0;
    g_cachedHoveredWidgetInventorySignature.clear();
    ClearLockedKeysetSource();
    ClearInventoryGuiInventoryLinks();
    ClearTraderPanelInventoryBindings();
}

bool TryInjectControlsToTarget(MyGUI::Widget* anchor, MyGUI::Widget* parent, const char* sourceTag)
{
    if (anchor == 0 || parent == 0)
    {
        LogErrorLine("could not resolve anchor/parent widget for controls injection");
        return false;
    }

    MyGUI::TabControl* tabs = FindOverviewTabs(parent);
    MyGUI::TabItem* craftingTab = GetActiveCraftingTab(tabs);
    if (craftingTab == 0)
    {
        std::stringstream line;
        line << "rejecting injection target reason=crafting_tab_inactive"
             << " source=" << (sourceTag == 0 ? "<unknown>" : sourceTag)
             << " anchor=" << SafeWidgetName(anchor)
             << " parent=" << SafeWidgetName(parent);
        if (ShouldLogDebug())
        {
            LogWarnLine(line.str());
        }
        return false;
    }

    const std::string nextTraderTargetId = BuildTraderTargetIdentity(anchor, parent);
    if (!nextTraderTargetId.empty() && !g_activeTraderTargetId.empty() && nextTraderTargetId != g_activeTraderTargetId)
    {
        g_loggedNumericOnlyQueryIgnored = false;
        g_lastSearchSampleQueryLogged.clear();
        g_lastZeroMatchQueryLogged.clear();
    }
    g_activeTraderTargetId = nextTraderTargetId;
    g_focusSearchEditOnNextInjection = g_autoFocusSearchInput;

    DestroyControlsIfPresent();
    SearchUiCallbacks callbacks;
    callbacks.onSearchTextChanged = &OnSearchTextChanged;
    callbacks.onSearchEditFocusChanged = &OnSearchEditKeyFocusChanged;
    callbacks.onSearchPlaceholderClicked = &OnSearchPlaceholderClicked;
    callbacks.onSearchClearButtonClicked = &OnSearchClearButtonClicked;
    if (!BuildControlsScaffold(tabs, craftingTab, callbacks))
    {
        LogErrorLine("failed to build phase 2 controls scaffold");
        return false;
    }

    g_controlsWereInjected = true;
    MarkSearchFilterDirty("controls_injected");
    ApplySearchFilterFromControls(false, ShouldLogSearchDebug());

    std::stringstream line;
    line << "controls scaffold injected"
         << " source=" << (sourceTag == 0 ? "<unknown>" : sourceTag)
         << " anchor=" << SafeWidgetName(anchor)
         << " parent=" << SafeWidgetName(parent);
    LogDebugLine(line.str());
    return true;
}

bool TryInjectControlsToHoveredWindowDirect()
{
    MyGUI::InputManager* inputManager = MyGUI::InputManager::getInstancePtr();
    if (inputManager == 0)
    {
        LogWarnLine("manual attach failed: MyGUI InputManager unavailable");
        return false;
    }
    if (inputManager->isModalAny())
    {
        return false;
    }

    MyGUI::Widget* hovered = inputManager->getMouseFocusWidget();
    if (hovered == 0)
    {
        LogWarnLine("manual attach failed: no mouse-focused widget; hover target window and press hotkey again");
        return false;
    }

    MyGUI::Widget* anchor = FindBestWindowAnchor(hovered);
    MyGUI::Widget* parent = ResolveInjectionParent(anchor);
    if (anchor == 0 || parent == 0)
    {
        std::stringstream line;
        line << "manual attach failed: anchor/parent unresolved hovered_chain=" << BuildParentChainForLog(hovered);
        LogWarnLine(line.str());
        return false;
    }

    if (!IsLikelyCraftingWindow(parent))
    {
        std::stringstream line;
        line << "manual attach failed: hovered window is not a crafting target"
             << " anchor=" << SafeWidgetName(anchor)
             << " parent=" << SafeWidgetName(parent)
             << " hovered_chain=" << BuildParentChainForLog(hovered);
        LogWarnLine(line.str());
        return false;
    }

    std::stringstream line;
    line << "manual attach using hovered crafting window"
         << " anchor=" << SafeWidgetName(anchor)
         << " parent=" << SafeWidgetName(parent)
         << " parent_coord=(" << parent->getCoord().left << "," << parent->getCoord().top << ","
         << parent->getCoord().width << "," << parent->getCoord().height << ")"
         << " hovered_chain=" << BuildParentChainForLog(hovered);
    LogDebugLine(line.str());

    if (ShouldLogDebug())
    {
        DumpHoveredAttachDiagnostics(hovered, anchor, parent);
    }

    return TryInjectControlsToTarget(anchor, parent, "crafting-hover-direct");
}

static void HideRecipePanel(MyGUI::Widget* panel, MyGUI::InputManager* input)
{
    FinalizeSearchContainerDrag("crafting_panel_hidden");
    ResetPendingSearchEditShortcut();
    g_pendingSlashFocusBaseQuery.clear();
    g_pendingSlashFocusTextSuppression = false;
    if (input != 0)
    {
        if (input->isCaptureMouse() && IsDescendantOf(input->getMouseFocusWidget(), panel))
        {
            input->resetMouseCaptureWidget();
        }
        if (IsDescendantOf(input->getKeyFocusWidget(), panel))
        {
            input->resetKeyFocusWidget();
        }
    }
    panel->setVisible(false);
}

static void RaiseRecipePanelIfCovered(MyGUI::Widget* panel)
{
    const MyGUI::ILayer* layer = panel->getLayer();
    const MyGUI::ILayerNode* node = panel->getLayerNode();
    MyGUI::LayerManager* layers = MyGUI::LayerManager::getInstancePtr();
    if (layer == 0 || node == 0 || layers == 0)
    {
        return;
    }

    // Raise only within Window so native Popup lists and higher dialogs keep priority.
    const std::size_t count = layer->getLayerNodeCount();
    if (count != 0 && layer->getLayerNodeAt(count - 1) != node)
    {
        layers->upLayerItem(panel);
    }
}

void EnsureControlsInjectedIfEnabled()
{
    if (!g_controlsEnabled)
    {
        return;
    }

    MyGUI::InputManager* input = MyGUI::InputManager::getInstancePtr();
    const bool hasModal = input != 0 && input->isModalAny();
    MyGUI::Widget* controlsContainer = FindControlsContainer();
    if (controlsContainer != 0 && g_recipePanel.tabs != 0 && g_recipePanel.craftingTab != 0)
    {
        if (hasModal || GetActiveCraftingTab(g_recipePanel.tabs) != g_recipePanel.craftingTab)
        {
            if (controlsContainer->getVisible())
            {
                HideRecipePanel(controlsContainer, input);
            }
            return;
        }

        if (!controlsContainer->getVisible())
        {
            controlsContainer->setVisible(true);
            MarkSearchFilterDirty("crafting_panel_shown");
        }
        RaiseRecipePanelIfCovered(controlsContainer);

        const MyGUI::IntSize viewport = GetRecipePanelViewport();
        if (viewport.width != g_recipePanel.viewport.width || viewport.height != g_recipePanel.viewport.height)
        {
            LayoutFloatingPanel();
        }
        return;
    }
    if (g_recipePanel.manager != 0)
    {
        ApplySearchFilterFromControls(true, false);
        DestroyControlsIfPresent();
    }
    if (hasModal)
    {
        return;
    }

    MyGUI::Widget* anchor = 0;
    MyGUI::Widget* parent = 0;
    if (TryResolveVisibleCraftingTarget(&anchor, &parent))
    {
        g_loggedNoVisibleTraderTarget = false;
        if (!TryInjectControlsToTarget(anchor, parent, "crafting-auto"))
        {
            LogWarnLine("crafting auto controls scaffold injection failed");
        }
        return;
    }

    if (!g_loggedNoVisibleTraderTarget)
    {
        if (ShouldLogDebug())
        {
            LogDebugLine("controls enabled but no visible crafting target found yet");
            DumpVisibleCraftingWindowCandidateDiagnostics();
        }
        g_loggedNoVisibleTraderTarget = true;
    }
}

void ApplyRuntimeSearchUiConfig(const char* reason)
{
    const bool hadControls = FindControlsContainer() != 0;
    if (!g_controlsEnabled)
    {
        if (hadControls)
        {
            ApplySearchFilterFromControls(true, false);
            DestroyControlsIfPresent();
        }

        g_loggedNoVisibleTraderTarget = false;
        if (ShouldLogDebug())
        {
            std::stringstream line;
            line << "runtime search ui config applied"
                 << " reason=" << (reason == 0 ? "<unknown>" : reason)
                 << " enabled=false"
                 << " had_controls=" << (hadControls ? "true" : "false");
            LogDebugLine(line.str());
        }
        return;
    }

    if (hadControls)
    {
        DestroyControlsIfPresent();
    }

    g_loggedNoVisibleTraderTarget = false;
    EnsureControlsInjectedIfEnabled();

    if (ShouldLogDebug())
    {
        const bool hasControls = FindControlsContainer() != 0;
        std::stringstream line;
        line << "runtime search ui config applied"
             << " reason=" << (reason == 0 ? "<unknown>" : reason)
             << " enabled=true"
             << " rebuilt=" << (hadControls ? "true" : "false")
             << " controls_present=" << (hasControls ? "true" : "false");
        LogDebugLine(line.str());
    }
}

bool IsToggleHotkeyPressedEdge()
{
    if (key == 0 || key->keyboard == 0)
    {
        g_prevToggleHotkeyDown = false;
        return false;
    }

    const bool ctrlDown = key->keyboard->isKeyDown(OIS::KC_LCONTROL)
        || key->keyboard->isKeyDown(OIS::KC_RCONTROL);
    const bool shiftDown = key->keyboard->isKeyDown(OIS::KC_LSHIFT)
        || key->keyboard->isKeyDown(OIS::KC_RSHIFT);
    const bool f8Down = key->keyboard->isKeyDown(OIS::KC_F8);

    const bool chordDown = ctrlDown && shiftDown && f8Down;
    const bool pressedEdge = chordDown && !g_prevToggleHotkeyDown;
    g_prevToggleHotkeyDown = chordDown;
    return pressedEdge;
}

bool IsDiagnosticsHotkeyPressedEdge()
{
    if (key == 0 || key->keyboard == 0)
    {
        g_prevDiagnosticsHotkeyDown = false;
        return false;
    }

    const bool ctrlDown = key->keyboard->isKeyDown(OIS::KC_LCONTROL)
        || key->keyboard->isKeyDown(OIS::KC_RCONTROL);
    const bool shiftDown = key->keyboard->isKeyDown(OIS::KC_LSHIFT)
        || key->keyboard->isKeyDown(OIS::KC_RSHIFT);
    const bool f9Down = key->keyboard->isKeyDown(OIS::KC_F9);

    const bool chordDown = ctrlDown && shiftDown && f9Down;
    const bool pressedEdge = chordDown && !g_prevDiagnosticsHotkeyDown;
    g_prevDiagnosticsHotkeyDown = chordDown;
    return pressedEdge;
}

void TickPhase2ControlsScaffold()
{
    if (IsDiagnosticsHotkeyPressedEdge())
    {
        DumpOnDemandTraderDiagnosticsSnapshot();
    }

    if (IsToggleHotkeyPressedEdge())
    {
        if (g_controlsEnabled)
        {
            ApplySearchFilterFromControls(true, false);
            g_controlsEnabled = false;
            DestroyControlsIfPresent();
            g_loggedNoVisibleTraderTarget = false;
            LogDebugLine("controls toggled OFF");
            return;
        }

        g_controlsEnabled = true;
        LogDebugLine("controls toggled ON");

        g_loggedNoVisibleTraderTarget = false;
        EnsureControlsInjectedIfEnabled();
        if (FindControlsContainer() == 0)
        {
            if (!TryInjectControlsToHoveredWindowDirect())
            {
                LogWarnLine("manual attach did not inject controls");
            }
        }
    }

    if (!g_controlsEnabled)
    {
        return;
    }

    EnsureControlsInjectedIfEnabled();
    MyGUI::Widget* panel = FindControlsContainer();
    if (panel != 0 && !panel->getInheritedVisible())
    {
        return;
    }
    TickSearchContainerDrag();
    bool handledSearchShortcut = false;
    MyGUI::EditBox* searchEdit = FindSearchEditBox();
    if (searchEdit != 0)
    {
        const SearchFocusHotkeyKind hotkeyKind =
            DetectSearchFocusHotkeyPressedEdge(searchEdit);
        if (hotkeyKind != SearchFocusHotkeyKind_None)
        {
            if (hotkeyKind == SearchFocusHotkeyKind_Slash)
            {
                g_pendingSlashFocusBaseQuery = g_searchQueryRaw;
                g_pendingSlashFocusTextSuppression = true;
            }
            else
            {
                g_pendingSlashFocusBaseQuery.clear();
                g_pendingSlashFocusTextSuppression = false;
            }
            FocusSearchEdit(searchEdit, "focus_hotkey");
            handledSearchShortcut = true;
        }
    }
    else
    {
        g_prevSearchSlashHotkeyDown = false;
        g_prevSearchCtrlFHotkeyDown = false;
        g_pendingSlashFocusBaseQuery.clear();
        g_pendingSlashFocusTextSuppression = false;
    }

    if (FindControlsContainer() != 0)
    {
        ObserveCraftingRecipeRowsForRefresh();
        ObserveTraderEntriesStateForRefresh();
    }

    if (FindControlsContainer() != 0 && !handledSearchShortcut && g_searchFilterDirty)
    {
        ApplySearchFilterFromControls(false, ShouldLogSearchDebug());
    }

    if (g_controlsWereInjected && FindControlsContainer() == 0)
    {
        g_controlsWereInjected = false;
        g_cachedHoveredWidgetInventory = 0;
        g_cachedHoveredWidgetInventorySignature.clear();
        ClearLockedKeysetSource();
        ClearInventoryGuiInventoryLinks();
        ClearTraderPanelInventoryBindings();
        LogDebugLine("controls container no longer present (window likely closed/destroyed); hover target window and press Ctrl+Shift+F8 to attach again");
    }
}
