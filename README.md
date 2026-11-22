## Organize the Crafting Stations

RE_Kenshi plugin that adds a reusable search bar to Kenshi crafting and research windows.

## Current behavior

- Opens a floating search and filter panel for detected crafting and research windows.
- Filters visible recipe and blueprint rows by substring match.
- Adds equipment category, armor class, and body-part coverage filters to the selected station's recipe list.
- Sorts native recipe rows by name, weapon stats, Standard Grade armor resistance, or coverage for the selected part.
- Compacts visible matches upward so filtering does not leave empty scroll gaps.
- Shows visible and total entry counts.
- Supports a clear button, configurable autofocus, and a floating panel that saves its position and dimensions.
- Applies Mod Hub setting changes live.
- Coexists with `Organize-the-Trader` without taking over trader windows.

## Equipment filters

Kenshi writes the output record ID and optional variant ID to each native recipe button. The plugin matches that pair against the station's available recipes from KenshiLib. It reads the weapon `skill category` and armor `slot` and `class` from the loaded game records. The choices include equipment added by content mods alongside vanilla equipment.

`All Recipes` includes entries with missing metadata. Duplicate captions remain independent when Kenshi displays separate rows. Layouts that replace the native recipe buttons can still use text search. If the research update hook is unavailable, switch stations or reopen the Crafting tab to refresh the available recipes after research.

### Body-part coverage

The part choices come from the loaded armor recipes' `part coverage` references. Choose a body part and enter a minimum percentage between 0 and 100. The filter applies to armor only and combines with the category, armor class, and search filters. Armor with no coverage for the selected part sorts as 0%. Choose **Any Body Part** to clear the coverage filter. With a part selected, choose **Coverage (high)** or **Coverage (low)** in **Sort**.

## Floating panel

Drag **Crafting filters** to move the panel anywhere in the game viewport. Use **−** to collapse it and **+** or **Ctrl+F** to expand it. Filters and sorting still apply while the panel is collapsed. The panel opens beside Crafting when space permits and starts collapsed if it needs a smaller footprint. It hides when the associated tab closes or a game modal is open.

The panel starts at about 350 pixels wide and uses the Crafting window skins. Drag the **//** grip at the bottom right to resize it. The search field, **X** button, and count have separate space throughout the supported size range. Dragging and resizing save the panel's screen position and size to the plugin's `mod-config.json`. Positions saved relative to the old Crafting window use the default placement until you drag the panel.

Each dropdown has its own row. Armor stations show **Slot** and **Class** when applicable, weapon stations show **Weapon**, and mixed stations show **Category**. **Coverage** and **Sort** have their own labels. The panel uses the stock font to measure the longest available option and reserves space for the dropdown arrow and scroll bar. The viewport limits how wide the panel can grow.

## Recipe sorting

**Original Order** is the default. **Name A–Z** and **Name Z–A** sort the visible matches by recipe caption. Sorting moves each native row together with its existing click and description handlers. Equal sort values retain the native row order.

Weapon stations offer highest and lowest first for cutting damage, blunt damage, and armor penetration. The preview uses the loaded recipe's weapon and model IDs with Kenshi's player manufacturer record. This compares the recipe grade shown in Crafting. A finished weapon can have a different grade depending on the crafter and station. Crossbow damage and weapons without a resolved model have no value for these sort modes.

Stations with armor recipes offer highest and lowest first for blunt resistance, cut resistance, and cut resistance efficiency (**Cut efficiency** in the menu). The plugin asks Kenshi's item factory for the same Standard Grade preview as the crafting description. The game exposes cut efficiency as the complement of the cut-to-stun fraction and displays it when the item provides cut resistance. Both weapon and armor sorting compare numerical values before tooltip rounding. Recipes without a value for the selected stat sort last in either direction.

## Overview resize investigation

Live Overview resizing is deferred. MyGUI resolves `position_real` to pixels when it loads `Kenshi_OverviewWindow.layout`. After that, each child's `align` flags control how it responds to changes in the parent size. The outer tab control stretches, but the Crafting and Research panes have no direct stretch anchors. Both AI panes stretch vertically from fixed top positions. In a model using the stock coordinates, increasing a 662-pixel tab by 160 pixels makes the two AI panes overlap by 150 pixels. [MyGUI layout creation](https://github.com/MyGUI/mygui/blob/MyGUI3.2.2/MyGUIEngine/src/MyGUI_ResourceLayout.cpp#L106-L134) · [MyGUI resize alignment](https://github.com/MyGUI/mygui/blob/MyGUI3.2.2/MyGUIEngine/src/MyGUI_Widget.cpp#L531-L590)

The native controllers also own geometry outside the layout. Kenshi initializes the map's zoom bounds and Faction's relation-row width during setup. Squads uses native cell dimensions and drag-and-drop targets. Crafting and Research use runtime data panels for their rows and descriptions. KenshiLib exposes resize or refresh functions for individual components. A live resizer would need to reflow each tab and validate its scroll state, selection, and hit targets separately.

A safer Overview experiment is an opt-in fixed-size layout loaded before Kenshi initializes those controllers. The independent **Crafting filters** panel provides adjustable space for the current build.

## Version 4 in-game checks

Copy the staged **Organize-the-Crafting-Stations-Version4** folder into the test installation's `mods` directory. Enable it with RE_Kenshi as the only copy of this plugin in that load order. Repeat the checks in vanilla and Genesis.

| Tab | Check |
| --- | --- |
| Crafting | At armor and weapon stations, resize the floating panel to its minimum and back. Check each label, the selected value, the longest dropdown option, and the search count and **X**. Combine category, class, coverage, and stat sorting where available. Hover and queue duplicate names or variants, then clear the filters and check the original list and queue order. |
| Map | Pan, zoom, and center the map. Verify markers and map hit targets after visiting Crafting. |
| Faction | Scroll relations and select a faction. Verify that the information pane follows the selected row. |
| Research | Switch types, search blueprints, and open a description. Verify that the research queue scrolls and the crafting choices refresh after an unlock. |
| Squads | Scroll and select squads and characters. Check drag-and-drop between squads in a test save. |
| Dialogue | Scroll the conversation log and verify that wrapped lines and the scroll bar work. |
| AI | Scroll options and check their tooltips wherever the tab is available. |

Collapse and expand **Crafting filters**, then restart Kenshi and check its saved position and size. For a missing recipe category, **Ctrl+Shift+F9** logs recipe bindings in `RE_Kenshi_log.txt`.

## Configuration

Settings live in [Organize-the-Crafting-Stations/mod-config.json](/mnt/i/Kenshi_modding/mods/Organize-the-Crafting-Stations/Organize-the-Crafting-Stations/mod-config.json).
The same enabled, count, clear button, autofocus, and size settings are exposed in Mod Hub when `Emkejs-Mod-Core` is loaded. Changing the width in Mod Hub replaces the width saved by the resize grip.

Shipped defaults

- `enabled`: `true`
- `showSearchEntryCount`: `true`
- `showSearchClearButton`: `true`
- `autoFocusSearchInput`: `true`
- `searchInputWidth`: `220`
- `searchInputHeight`: `26`
- `searchPanelWidth` and `searchPanelHeight` use automatic sizing when set to `0`.

The authoritative implementation plan is:
- [docs/plans/main_plan.md](/mnt/i/Kenshi_modding/mods/Organize-the-Crafting-Stations/docs/plans/main_plan.md)

The authoritative developer role is:
- [docs/roles/dev.md](/mnt/i/Kenshi_modding/mods/Organize-the-Crafting-Stations/docs/roles/dev.md)

## Cleanup policy for this repo

- Keep reusable search-input pieces.
- Remove stale trader-only docs and planning residue.
- Do not trust copied trader placement or binding assumptions for crafting.
- Keep external build and deploy identity aligned to `Organize-the-Crafting-Stations`.

## License
This project is licensed under the GNU General Public License v3.0.
It uses KenshiLib, which is released under GPLv3.
