# Kenshi Advanced Crafting Filters v1.0.1

A Kenshi crafting plugin with text search, a floating filter panel, and recipe sorting. Equipment categories, armor slots and classes, and body-part choices come from recipes available at the selected station and the game's loaded equipment records.

## Requirements

- A 64-bit Windows Kenshi installation supported by [RE_Kenshi 0.3.5](https://github.com/BFrizzleFoShizzle/RE_Kenshi/releases/tag/v0.3.5), with that loader and its bundled KenshiLib installed. The Genesis test used the Steam installation with RE_Kenshi reporting Kenshi 1.0.65.
- The [Microsoft Visual C++ 2010 SP1 Redistributable (x64)](https://www.microsoft.com/en-us/download/details.aspx?id=26999) if the runtime is missing from your system.

## Installation

For a Workshop installation, subscribe to the mod and enable it in the Kenshi launcher. [Steam Workshop listing](https://steamcommunity.com/sharedfiles/filedetails/?id=3844312914)

For a manual installation, use the installation ZIP.

1. Close Kenshi. Follow the linked RE_Kenshi release instructions if the loader is missing. Its version should appear on the Kenshi main menu after installation.
2. Locate your Kenshi installation. In Steam, right-click **Kenshi** and choose **Manage → Browse local files**. For GOG or another standalone install, open the game folder containing `kenshi_x64.exe`.
3. Extract `Kenshi-Advanced-Crafting-Filters-v1.0.1.zip` into that installation's `mods` directory.

   ```text
   Kenshi/
     mods/
       Kenshi-Advanced-Crafting-Filters/
         Kenshi-Advanced-Crafting-Filters.mod
         Kenshi-Advanced-Crafting-Filters.dll
         RE_Kenshi.json
         mod-config.json
   ```

4. Enable **Kenshi-Advanced-Crafting-Filters** in the launcher and start Kenshi normally.

Important: Disable or uninstall the original Organize the Crafting Stations plugin before installing Kenshi Advanced Crafting Filters. Running both plugins simultaneously may cause conflicts.

Enable a single copy of Kenshi Advanced Crafting Filters.

## Using the filters

Open **Crafting** in the Overview and select a station. Drag the **Crafting filters** header to move the panel or its **//** corner grip to resize it. **−** collapses the panel. **+** or **Ctrl+F** expands it. The plugin saves the panel's position and size to `mod-config.json` in its mod folder.

| Control | Use |
| --- | --- |
| Search | Filter the recipe names by text. Use **X** to clear the query. |
| Weapon, Slot, or Category | Pick a weapon category or an armor slot. The label follows the station's available equipment. **All Recipes** clears this filter. |
| Class | Narrow armor to Cloth, Light, Medium, or Heavy where present. **All Classes** clears this filter. |
| Coverage | Pick a body part and enter a minimum percentage from 0 to 100. **Any Body Part** clears this filter. |
| Sort | Choose **Original Order**, **Name A–Z**, **Name Z–A**, or a numeric high or low order. The numeric choices follow the station's equipment and the selected body part. |

Weapon sorts use cutting damage, blunt damage, and armor penetration at the recipe's preview grade. Armor sorts use blunt resistance, cut resistance, and cut resistance efficiency at **Standard Grade**. Coverage sorts use the selected body part. Items without a value for the chosen stat sort last, and ties follow the original recipe order. Filters can be combined. The sort applies to available recipe rows only, and each row keeps its native recipe and queue action.

The plugin remembers your choices during the game session. If a station lacks a selected option, the panel uses the available default for that filter. The preferred choice returns when it becomes available again. Selecting a different option replaces the preference.

## Compatibility and limitations

The v1.0.1 panel visibility, window order, and filter persistence were tested in-game with **Full Screen Map (FSM)**. The panel stays above the Overview on Crafting and hides on other tabs. FSM is optional.

The crafting implementation shipped in v1.0.0 was tested with **Genesis**. Standard game records let the filters discover added equipment when other content mods use Kenshi's native crafting rows. Other UI overhauls may need separate compatibility checks.

The original plugin's Research-tab blueprint search shared the floating controls. Version 1.0.1 limits those controls to Crafting, so blueprint search is unavailable on the Research tab. Kenshi's native Research controls and the crafting recipe refresh after research still work as before.

Some UI replacements can leave only text search available. Weapon damage modes apply to melee weapon previews. Crossbows and unresolved weapon models have no comparable value in those modes. A finished weapon may have a different grade than its preview. If a newly researched recipe is missing, switch stations or reopen Crafting to refresh the list.

## Troubleshooting

- **The panel is missing.** Check that RE_Kenshi appears on the main menu and the release is checked in the launcher. Verify the folder structure above, close any modal dialog, and select a crafting station. **Ctrl+Shift+F8** toggles the filter controls.
- **Duplicate panels or a load conflict.** Ensure only Kenshi Advanced Crafting Filters is enabled. Disable the original Organize the Crafting Stations mod or any other plugin that modifies the same crafting interface.
- **A stat is unavailable.** Compare the recipe description and check that the station has the relevant equipment. To show every recipe again, clear the text, choose **All Recipes** and **Any Body Part**, and select **Original Order**.
- **The plugin fails to load.** Check `RE_Kenshi_log.txt` in the Kenshi installation for the plugin load result. Verify that the installer supplied KenshiLib and that the x64 Visual C++ 2010 runtime is installed. For recipe binding diagnostics, temporarily set `debugLogging` and `debugBindingLogging` to `true` in this mod's `mod-config.json` and press **Ctrl+Shift+F9** with Crafting open.

## Uninstallation

Close the game and disable **Kenshi-Advanced-Crafting-Filters** in the launcher. For a Workshop installation, unsubscribe from the item. For a manual installation, remove the mod's own folder from `mods`. Keep a copy of `mod-config.json` if you want to reuse your panel settings.

## Credits and source

Derived from Emkej's [Organize the Crafting Stations](https://github.com/Emkej/Organize-the-Crafting-Stations). The advanced equipment, coverage, sorting, and floating panel changes are part of this modified edition. [License](LICENSE) · [Attribution and source notice](NOTICE.md) · [Release notes](CHANGELOG.md)

The corresponding source is in `Kenshi-Advanced-Crafting-Filters-v1.0.1-source.zip`. It includes the build instructions and packaging scripts. [GitHub Releases](https://github.com/Lardhunter/Kenshi-Advanced-Crafting-Filters/releases)
