# Kenshi Advanced Crafting Filters v1.0.1 test build 2

A Kenshi crafting plugin with text search, a floating filter panel, and recipe sorting. Equipment categories, armor slots and classes, and body-part choices come from recipes available at the selected station and the game's loaded equipment records.

## Requirements

- A 64-bit Windows Kenshi installation supported by [RE_Kenshi 0.3.5](https://github.com/BFrizzleFoShizzle/RE_Kenshi/releases/tag/v0.3.5), with that loader and its bundled KenshiLib installed. The Genesis test used the Steam installation with RE_Kenshi reporting Kenshi 1.0.65.
- The [Microsoft Visual C++ 2010 SP1 Redistributable (x64)](https://www.microsoft.com/en-us/download/details.aspx?id=26999) if the runtime is missing from your system.

## Installation

1. Close Kenshi. Follow the linked RE_Kenshi release instructions if the loader is missing. Its version should appear on the Kenshi main menu after installation.
2. Locate your Kenshi installation. In Steam, right-click **Kenshi** and choose **Manage → Browse local files**. For GOG or another standalone install, open the game folder containing `kenshi_x64.exe`.
3. Extract `Kenshi-Advanced-Crafting-Filters-v1.0.1-test-2.zip` into that installation's `mods` directory. The test build uses its own mod folder.

   ```text
   Kenshi/
     mods/
       Kenshi-Advanced-Crafting-Filters-v1.0.1-test-2/
         Kenshi-Advanced-Crafting-Filters-v1.0.1-test-2.mod
         Kenshi-Advanced-Crafting-Filters.dll
         RE_Kenshi.json
         mod-config.json
   ```

Important: Disable or uninstall the original Organize the Crafting Stations plugin before installing Kenshi Advanced Crafting Filters. Running both plugins simultaneously may cause conflicts.

Disable the published v1.0.0 copy and the first v1.0.1 test copy in the launcher. Enable **Kenshi-Advanced-Crafting-Filters-v1.0.1-test-2** for the compatibility checks.

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

The crafting implementation shipped in v1.0.0 was tested with **Genesis**. The first v1.0.1 test confirmed the panel's visibility and window order with **Full Screen Map**. In-game validation of the filter persistence changes is pending. Standard game records let the filters discover added equipment when other content mods use Kenshi's native crafting rows.

Some UI replacements can leave only text search available. Weapon damage modes apply to melee weapon previews. Crossbows and unresolved weapon models have no comparable value in those modes. A finished weapon may have a different grade than its preview. If a newly researched recipe is missing, switch stations or reopen Crafting to refresh the list.

## Troubleshooting

- **The panel is missing.** Check that RE_Kenshi appears on the main menu and the release is checked in the launcher. Verify the folder structure above, close any modal dialog, and select a crafting station. **Ctrl+Shift+F8** toggles the filter controls.
- **Duplicate panels or a load conflict.** Ensure only Kenshi Advanced Crafting Filters is enabled. Disable the original Organize the Crafting Stations mod or any other plugin that modifies the same crafting interface.
- **A stat is unavailable.** Compare the recipe description and check that the station has the relevant equipment. To show every recipe again, clear the text, choose **All Recipes** and **Any Body Part**, and select **Original Order**.
- **The plugin fails to load.** Check `RE_Kenshi_log.txt` in the Kenshi installation for the plugin load result. Verify that the installer supplied KenshiLib and that the x64 Visual C++ 2010 runtime is installed. For recipe binding diagnostics, temporarily set `debugLogging` and `debugBindingLogging` to `true` in this mod's `mod-config.json` and press **Ctrl+Shift+F9** with Crafting open.

## Uninstallation

Close the game, disable **Kenshi-Advanced-Crafting-Filters-v1.0.1-test-2** in the launcher, and remove its folder from `mods`. You can re-enable the published v1.0.0 copy after disabling the test build.

## Credits and source

Derived from Emkej's [Organize the Crafting Stations](https://github.com/Emkej/Organize-the-Crafting-Stations). The advanced equipment, coverage, sorting, and floating panel changes are part of this modified edition. [License](LICENSE) · [Attribution and source notice](NOTICE.md) · [Release notes](CHANGELOG.md)

The corresponding source for this local test build is on the `fix/v1.0.1-fsm-compatibility` development branch. The project includes the build instructions and packaging scripts.
