# Release notes

## v1.0.1 (test build, 2026-11-23)

- Keep the floating panel on the selected Crafting tab.
- Preserve search, equipment categories, armor classes, coverage, and sorting through tab changes and recipe refreshes. Apply an All option when the current station lacks a preferred category.
- Raise the panel when another window moves ahead of it on the game’s Window layer. Native dropdowns, dialogs, and tooltips retain their priority.
- Handle the stock and Full Screen Map layouts through their shared MyGUI tab structure. Initial in-game testing with Full Screen Map confirmed the panel's visibility and window order.

## v1.0.0 (2026-11-22)

- Browse dynamic weapon categories and armor slots and classes from the selected station's available equipment.
- Move, resize, or collapse the independent filter panel. The panel saves its position and dimensions and labels its dropdowns by function.
- Combine text search with alphabetical or original recipe order.
- Sort melee weapons by cutting damage, blunt damage, or armor penetration at their recipe preview grade.
- Sort armor by blunt resistance, cut resistance, or cut resistance efficiency at Standard Grade.
- Filter by a minimum coverage percentage and sort by coverage for a single body part.
- Preserve native row identities, duplicate names, variants, and crafting queue order.

The Version 4 implementation passed in-game testing with Genesis on Steam Kenshi x64 with RE_Kenshi 0.3.5. Other content mods and UI replacements have not been certified.

Resizing Kenshi's main Overview window was investigated and deliberately excluded. The stock layout has fixed panes and overlapping resize anchors, and native tab controllers maintain their own geometry. The resizable crafting panel is included.
