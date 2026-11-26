# Kenshi Advanced Filters v2.0.0

Kenshi Advanced Filters, formerly **Kenshi Advanced Crafting Filters**, adds independent search and filter panels to the Crafting and Research tabs. Filter equipment, compare recipe stats, find research across categories, and browse technologies already learned in your save.

## Requirements

- A 64-bit Windows Kenshi installation supported by [RE_Kenshi 0.3.5](https://github.com/BFrizzleFoShizzle/RE_Kenshi/releases/tag/v0.3.5), with the loader and its bundled KenshiLib installed. The tested setup used Steam Kenshi 1.0.65 and RE_Kenshi 0.3.5.
- The [Microsoft Visual C++ 2010 SP1 Redistributable (x64)](https://www.microsoft.com/en-us/download/details.aspx?id=26999) if the runtime is missing.

## Install or upgrade

**Steam Workshop.** Subscribe to the existing listing and enable the mod in the Kenshi launcher. If you already subscribe, let Steam finish the update. The item ID and internal mod name stay the same. [Steam Workshop listing](https://steamcommunity.com/sharedfiles/filedetails/?id=3844312914)

**Manual installation.** Use the installation ZIP on Steam or a supported standalone installation.

1. Close Kenshi and install RE_Kenshi if needed. Its version appears on the Kenshi main menu when it is loaded.
2. Find your game folder. In Steam, right-click **Kenshi** and choose **Manage → Browse local files**. On GOG or another standalone installation, open the folder containing `kenshi_x64.exe`.
3. If you have a previous manual version, back up its `mod-config.json` outside `mods` to keep your settings. Extract `Kenshi-Advanced-Filters-v2.0.0.zip` into the game's `mods` directory and replace the prior manual files. Restore your saved `mod-config.json` if desired.

   ```text
   Kenshi/
     mods/
       Kenshi-Advanced-Crafting-Filters/
         Kenshi-Advanced-Crafting-Filters.mod
         Kenshi-Advanced-Crafting-Filters.dll
         RE_Kenshi.json
         mod-config.json
   ```

4. Enable **Kenshi-Advanced-Crafting-Filters** in the launcher and start Kenshi. The launcher can still show the original internal name.

Use a single enabled copy, either Workshop or manual. Disable the original **Organize the Crafting Stations** plugin and any older test copies before starting the game.

## Using the panels

Open **Crafting** or **Research** in the Overview. Drag the panel header to move the active panel or drag its corner grip to resize it. Use **−** to collapse it and **+** or **Ctrl+F** to expand it. Each panel saves its own position and dimensions to `mod-config.json`. Search and filter selections are independent and persist through tab changes during the game session. **X** clears the search. **Clear Filters** restores the active tab's default filters while preserving its panel geometry and the other tab's filters.

### Crafting

Select a station. The panel discovers equipment categories and body parts from available recipes and the game's loaded records, including equipment supplied by content mods.

| Control | Use |
| --- | --- |
| Search | Narrow the available recipe names by text. |
| Weapon, Slot, or Category | Pick a weapon category or an equipment slot offered by the station. |
| Class | Filter armor by Cloth, Light, Medium, or Heavy when present. |
| Coverage | Choose a body part and enter a minimum coverage percentage from 0 to 100. |
| Sort | Use original order, alphabetical order, or a numeric comparison. Options depend on the station and selected body part. |

- **Equipment stats** include melee cutting damage, blunt damage, and armor penetration, plus armor blunt resistance, cut resistance, cut resistance efficiency, and coverage for the selected body part.
- **Item Value**, **Sell Value**, and **Craft Time** compare native single-item preview values and estimated craft hours. Weapon previews use the recipe's model grade. Armor uses Standard Grade. An actual item can finish at a different quality or take a different amount of time.
- **Unit sell value/hr** divides one item's preview sell value by its estimated craft hours. This measure excludes material costs, batch output, market adjustments, and the particular crafter's or station's speed.

You can combine the filters and sorting. Items without a value for the chosen stat go last, and ties keep native order. The list keeps native recipe identities and queue actions. If a station lacks a preferred option, the panel uses the available default until that preference becomes applicable again or you choose a different one.

### Research

Search by technology name. Use **Type** to choose how widely to search.

- **Auto (All Types on search)** uses the current type with an empty query and searches across types as you type.
- **All Types** combines the categories even with an empty query.
- **Current Type** follows the category selected in Kenshi's Types list.

Use **Status** to browse **Available**, **Learned**, or **All Research**. Learned reads the game's completed research from your loaded save, including technologies completed in previous sessions. All Research also includes other loaded definitions and labels unavailable entries. Kenshi applies its usual requirements when you queue research.

Select an available result in the floating list to open its native category and preview the exact technology. Use the native row to add it to the queue. Learned and unavailable results show read-only details, including prerequisites and unlocks when supplied by the definition. **Clear Filters** restores Auto and Available.

## Compatibility and limitations

- The Crafting filters have been tested with **Genesis**. Panel visibility and window order have been tested with **Full Screen Map**. Both mods are optional. Other content mods can supply recipes and research through Kenshi's native records. UI overhauls that replace the underlying controls may limit filtering or navigation.
- Weapon damage sorts apply to melee previews. Crossbows and unresolved weapon models can have missing values. Unit sell value per hour is unavailable if estimated time is missing or zero.
- Learned can show only definitions still loaded and marked complete by the game. Blueprint or starting unlocks appear when Kenshi records their research as complete. Research kept after a save import follows Kenshi's import settings. If the installed runtime lacks the catalog functions, use **Current Type** and native research browsing.
- Localization remains a possible future improvement.

## Troubleshooting

- **The panel is missing.** Confirm RE_Kenshi appears on the main menu and the mod is enabled. Open Crafting or Research, close modal dialogs, and press **Ctrl+F** if the panel is collapsed. **Ctrl+Shift+F8** toggles the filter controls.
- **Duplicate panels.** Check that only one Workshop or manual copy is enabled and that the original Organize the Crafting Stations plugin is disabled.
- **A recipe or technology is missing.** Use **Clear Filters** and confirm the station or technology is available in the game. Try reopening the relevant tab after research completes. Use **All Types** and **Learned** to view previously completed technologies.
- **The plugin fails to load.** Check `RE_Kenshi_log.txt` in the game folder. Confirm KenshiLib was supplied by RE_Kenshi and the x64 Visual C++ 2010 runtime is available.

## Uninstall

Close Kenshi and disable **Kenshi-Advanced-Crafting-Filters** in the launcher. Unsubscribe for a Workshop installation or remove the mod's own folder for a manual installation. Back up `mod-config.json` if you want to reuse your panel settings.

## Credits and source

This GPLv3 mod is derived from Emkej's **Organize the Crafting Stations**. It preserves the upstream copyright and attribution. [Original project](https://github.com/Emkej/Organize-the-Crafting-Stations) · [License](LICENSE) · [Attribution](NOTICE.md) · [Changelog](CHANGELOG.md)

The corresponding source archive, `Kenshi-Advanced-Filters-v2.0.0-source.zip`, includes the build instructions and release scripts. [GitHub Releases](https://github.com/Lardhunter/Kenshi-Advanced-Crafting-Filters/releases)
