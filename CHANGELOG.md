# Release notes

## v2.0.0

- Rename the public mod to **Kenshi Advanced Filters**. The existing Workshop listing and internal mod names continue to identify the update for subscribers and manual installations.
- Add an independent floating Research panel with cross-category search and **Auto**, **All Types**, and **Current Type** views.
- Browse **Available**, **Learned**, and **All Research**. Learned includes completed records from a loaded save. Learned and unavailable results show details, prerequisites, and unlocks when provided by the game records.
- Open an available result in its native category by research ID. Research and crafting queue actions remain in the native interface.
- Add Crafting sorts for **Item Value**, **Sell Value**, **Craft Time**, and **Unit sell value/hr**. Use the game's recipe previews and keep native order for ties.
- Keep separate search, filter, and panel settings for Crafting and Research. **Clear Filters** resets the active tab, and the panel hides when the player leaves that tab.
- Retain dynamic equipment and body-part filters, weapon and armor stat sorting, and the window behavior tested with Full Screen Map.

## v1.0.1 (2026-11-23)

- Keep the floating panel above Crafting and hide it when another Overview tab is selected. Native dropdowns, dialogs, and tooltips retain their priority.
- Preserve text search, weapon category, armor slot and class, coverage, and sorting through tab changes and recipe refreshes during the game session. Use the available default when a station lacks a preferred option.
- Fix compatibility with Full Screen Map through the standard Overview tab structure. Panel visibility, window order, and retained filters passed in-game testing.
- Limit the shared search controls to Crafting. The original plugin's Research-tab blueprint search is excluded from this version.

## v1.0.0 (2026-11-22)

- Browse dynamic weapon categories and armor slots and classes from the selected station's available equipment.
- Move, resize, or collapse the independent filter panel. The panel saves its position and dimensions and labels its dropdowns by function.
- Combine text search with alphabetical or original recipe order.
- Sort melee weapons by cutting damage, blunt damage, or armor penetration at their recipe preview grade.
- Sort armor by blunt resistance, cut resistance, or cut resistance efficiency at Standard Grade.
- Filter by a minimum coverage percentage and sort by coverage for a single body part.
- Preserve native row identities, duplicate names, variants, and crafting queue order.

The v1.0.0 Crafting features passed in-game testing with Genesis on Steam Kenshi x64 with RE_Kenshi 0.3.5. Other content mods and UI replacements may require separate compatibility checks.

Resizing Kenshi's main Overview window was investigated and deliberately excluded. The stock layout has fixed panes and overlapping resize anchors, and native tab controllers maintain their own geometry. The resizable crafting panel is included.
