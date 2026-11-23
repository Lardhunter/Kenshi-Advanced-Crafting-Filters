# Attribution and source notice

**Kenshi Advanced Crafting Filters** is a modified edition of **Organize the Crafting Stations** by Emkej and its contributors. It is distributed under the GNU General Public License, version 3, as set out in the accompanying `LICENSE`. The upstream license text is reproduced in full. [Original project](https://github.com/Emkej/Organize-the-Crafting-Stations)

The modifications recorded for this edition on 2026-11-22 add the floating filter panel, equipment and body-part filters, and recipe sorting. The upstream search and integration code, authorship records, and license remain part of the project history. The Mod Hub adapter was inherited from the upstream project and uses Emkej's Emkejs-Mod-Core SDK. [Emkejs-Mod-Core](https://github.com/Emkej/Emkejs-Mod-Core)

Version 1.0.1, dated 2026-11-23, updates the floating panel's visibility, window order, and filter persistence.

## Corresponding source

Distribute `Kenshi-Advanced-Crafting-Filters-v1.0.1-source.zip` alongside `Kenshi-Advanced-Crafting-Filters-v1.0.1.zip`. The source bundle includes the plugin sources, project files, mod template, and scripts to build and package this version. When offering the binary for download, make its corresponding source available in the same way at no additional charge. [GNU GPLv3, section 6](https://www.gnu.org/licenses/gpl-3.0.html#section6)

## External components

- RE_Kenshi 0.3.5 supplies the plugin loader and runtime KenshiLib. Upstream provides the loader source, license, and installation instructions. [RE_Kenshi 0.3.5](https://github.com/BFrizzleFoShizzle/RE_Kenshi/releases/tag/v0.3.5) · [RE_Kenshi source](https://github.com/BFrizzleFoShizzle/RE_Kenshi/tree/v0.3.5)
- The build uses the KenshiLib 0.4.0 headers and import libraries from the KenshiLib example dependency checkout pinned in `BUILD.md`. KenshiLib is GPLv3 and retains its upstream third-party notices. [KenshiLib source and notices](https://github.com/BFrizzleFoShizzle/KenshiLib/tree/v0.4.0) · [KenshiLib example dependencies](https://github.com/BFrizzleFoShizzle/KenshiLib_Examples_deps)
- Boost 1.60.0 is used under the Boost Software License 1.0. [Boost 1.60.0 source](https://www.boost.org/releases/1.60.0/) · [Boost Software License 1.0](https://www.boost.org/LICENSE_1_0.txt)
- The game installation supplies the MyGUI and Ogre runtime libraries and the interface assets. The build SDK supplies their import libraries. [MyGUI source](https://github.com/MyGUI/mygui) · [OGRE source](https://github.com/OGRECave/ogre)
