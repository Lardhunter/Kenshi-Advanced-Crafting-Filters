# Kenshi Advanced Crafting Filters v1.0.0

A Kenshi crafting and research plugin with text search, a floating filter panel, and recipe sorting. Equipment categories, armor slots and classes, and body-part choices come from recipes available at the selected station and the game's loaded equipment records.

## Requirements

- A 64-bit Windows Kenshi installation supported by [RE_Kenshi 0.3.5](https://github.com/BFrizzleFoShizzle/RE_Kenshi/releases/tag/v0.3.5), with that loader and its bundled KenshiLib installed. The Genesis test used the Steam installation with RE_Kenshi reporting Kenshi 1.0.65.
- The [Microsoft Visual C++ 2010 SP1 Redistributable (x64)](https://www.microsoft.com/en-us/download/details.aspx?id=26999) if the runtime is missing from your system.

## Installation

1. Close Kenshi. Follow the linked RE_Kenshi release instructions if the loader is missing. Its version should appear on the Kenshi main menu after installation.
2. Locate your Kenshi installation. In Steam, right-click **Kenshi** and choose **Manage → Browse local files**. For GOG or another standalone install, open the game folder containing `kenshi_x64.exe`.
3. Extract `Kenshi-Advanced-Crafting-Filters-v1.0.0.zip` into that installation's `mods` directory. The resulting structure should include these files directly inside the mod folder.

   ```text
   Kenshi/
     mods/
       Kenshi-Advanced-Crafting-Filters/
         Kenshi-Advanced-Crafting-Filters.mod
         Kenshi-Advanced-Crafting-Filters.dll
         RE_Kenshi.json
         mod-config.json
   ```

4. In the Kenshi launcher's **Mods** list, disable **Organize the Crafting Stations** and any earlier test copy of this plugin, such as **Phase3** or **Version4**. Disable a Workshop copy of the original plugin in the same way if it appears there. Enable **Kenshi-Advanced-Crafting-Filters** and start Kenshi normally.

## Using the filters

Open **Crafting** in the Overview and select a station. Drag the **Crafting filters** header to move the panel or its **//** corner grip to resize it. **−** collapses the panel. **+** or **Ctrl+F** expands it. The plugin saves the panel's position and size to `mod-config.json` in its mod folder.

| Control | Use |
| --- | --- |
| Search | Filter the recipe names by text. Use **X** to clear the query. The same search works on research blueprints. |
| Weapon, Slot, or Category | Pick a weapon category or an armor slot. The label follows the station's available equipment. **All Recipes** clears this filter. |
| Class | Narrow armor to Cloth, Light, Medium, or Heavy where present. **All Classes** clears this filter. |
| Coverage | Pick a body part and enter a minimum percentage from 0 to 100. **Any Body Part** clears this filter. |
| Sort | Choose **Original Order**, **Name A–Z**, **Name Z–A**, or a numeric high or low order. The numeric choices follow the station's equipment and the selected body part. |

Weapon sorts use cutting damage, blunt damage, and armor penetration at the recipe's preview grade. Armor sorts use blunt resistance, cut resistance, and cut resistance efficiency at **Standard Grade**. Coverage sorts use the selected body part. Items without a value for the chosen stat sort last, and ties follow the original recipe order. Filters can be combined. The sort applies to available recipe rows only, and each row keeps its native recipe and queue action.

## Compatibility and limitations

The Version 4 implementation was tested in-game with **Genesis**. Version 1.0.0 packages that implementation. Standard game records let the filters discover added equipment when other content mods use Kenshi's native crafting rows. Other overhauls, UI replacements, and standalone installations remain unverified for this release.

Some UI replacements can leave only text search available. Weapon damage modes apply to melee weapon previews. Crossbows and unresolved weapon models have no comparable value in those modes. A finished weapon may have a different grade than its preview. If a newly researched recipe is missing, switch stations or reopen Crafting to refresh the list.

## Troubleshooting

- **The panel is missing.** Check that RE_Kenshi appears on the main menu and the release is checked in the launcher. Verify the folder structure above, close any modal dialog, and select a crafting station. **Ctrl+Shift+F8** toggles the filter controls.
- **Duplicate panels or a load conflict.** Enable just one copy of the crafting plugin in the launcher. Check for the original mod, a Workshop copy, and older test copies.
- **A stat is unavailable.** Compare the recipe description and check that the station has the relevant equipment. To show every recipe again, clear the text, choose **All Recipes** and **Any Body Part**, and select **Original Order**.
- **The plugin fails to load.** Check `RE_Kenshi_log.txt` in the Kenshi installation for the plugin load result. Verify that the installer supplied KenshiLib and that the x64 Visual C++ 2010 runtime is installed. For recipe binding diagnostics, temporarily set `debugLogging` and `debugBindingLogging` to `true` in this mod's `mod-config.json` and press **Ctrl+Shift+F9** with Crafting open.

## Uninstallation

Close the game, disable **Kenshi-Advanced-Crafting-Filters** in the launcher, and remove its own folder from `mods`. Keep a copy of its `mod-config.json` if you want to reuse your panel settings. You can re-enable the original plugin after disabling this release.

## Credits and source

Derived from Emkej's [Organize the Crafting Stations](https://github.com/Emkej/Organize-the-Crafting-Stations). The advanced equipment, coverage, sorting, and floating panel changes are part of this modified edition. [License](LICENSE) · [Attribution and source notice](NOTICE.md) · [Release notes](CHANGELOG.md)

The companion archive `Kenshi-Advanced-Crafting-Filters-v1.0.0-source.zip` contains the corresponding plugin source, build instructions, and release packaging scripts.
