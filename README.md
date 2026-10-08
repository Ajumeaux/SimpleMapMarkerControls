# Simple Map Marker Controls

Lightweight SKSE plugin providing simpler mouse controls for Skyrim's custom player map marker.

The plugin does not replace or modify map SWF files, marker assets, marker records, visuals, or map data.

## Features

### World Map

- Left-click to place the custom destination marker.
- Left-click elsewhere to move it immediately.
- Left-click the custom destination marker to remove it.
- Normal map markers keep their vanilla behavior.

### Local Map

Vanilla Skyrim does not support custom player markers on the Local Map.

If [Local Map Upgrade](https://www.nexusmods.com/skyrimspecialedition/mods/129756) is installed, the same controls are enabled for its Local Map marker support.

Source:
[alexsylex/LocalMapUpgrade](https://github.com/alexsylex/LocalMapUpgrade)

Without Local Map Upgrade, the vanilla Local Map is left unchanged.

## Compatibility

Tested on:

- Skyrim 1.6.1170
- Skyrim 1.7.x

Local Map integration depends on the Skyrim versions supported by Local Map Upgrade.

Built with CommonLibSSE-NG and Address Library.

Licensed under GPL-3.0-or-later.