# UNI2 Block List

[中文说明](README.zh-CN.md)

A Windows tool for blocking unwanted players in **UNDER NIGHT IN-BIRTH II Sys:Celes** Steam matchmaking.

[**Download for Windows**](https://github.com/geturin/uni2-block-list/releases/download/v0.1.0-rc.1/UNI2-Block-List-0.1.0-rc.1-win32.zip) · [All releases](https://github.com/geturin/uni2-block-list/releases)

## Features

- Show recently observed matchmaking players, names, estimated latency and connection type.
- Block a selected player or unblock them from your saved block list.
- **Block Wi-Fi players** without adding each player to the saved list.
- View skipped, rejected, intercepted and dropped request counts.
- Switch between English and Chinese. The first launch follows the Windows UI language: Chinese for Chinese systems, English otherwise.

## Getting started

Requires Windows 10 or 11, a supported Steam version of the game and your own game installation. The portable download needs no Python or installer.

1. Extract the **whole ZIP** into a folder. Keep `UNI2 Block List.exe` and `uni2-block-list.dll` together.
2. Start the game normally through Steam.
3. Run `UNI2 Block List.exe` with the same user and permissions as the game.
4. Click **Refresh**, select the game's process, then click **Connect**. Leave **Enable filtering** checked.
5. Enter the game's matchmaking search or standby mode. Select a player and click **Block player**. Select a saved player and click **Unblock** to remove them.

**Block Wi-Fi players** applies while checked; it does not add players to your saved list. Unknown connection types are allowed. You can change the language in the upper-right menu; your choice is saved.

To stop filtering, uncheck **Enable filtering** or close the tool. **Before upgrading, close the tool and restart the game**, then extract the new package. The injected DLL remains loaded until the game exits.

## What the list and counters mean

The player list reflects results the game has actually observed, not a complete Steam matchmaking queue. Recently seen players remain selectable for up to two minutes; they may no longer be waiting. Names, latency and connection type can be unknown when the game has not provided them. Latency is an estimate, not an independent network measurement.

Filtering prevents supported matchmaking requests from reaching a blocked player. It pauses during an active battle and does not cancel an ongoing match. **Request activity** shows changes in the tool's real counters, grouped by observation time. These are not packet-loss measurements or a per-packet network trace.

<details>
<summary>Supported game version</summary>

The tool verifies the game files and refuses an unsupported version. A game update may require a tool update. The current supported SHA-256 fingerprints are:

| File | SHA-256 |
| --- | --- |
| `uni2.exe` | `4ebed985ecbf330ab8e495573361e49df20bb555263289d1aff5425fac9b7ed9` |
| `steam_api.dll` | `67ae11d71ae6ec404090094df1e47b614d27400dc53fa450023e6fbcf347902c` |

</details>

## Settings and privacy

The block list and preferences are saved locally in `%LOCALAPPDATA%\UNI2BlockList`. Saved players and the Wi-Fi option from the previous tool are imported when the corresponding new setting is absent. Its language preference is not imported; the first public launch uses the Windows UI language.

The tool does not automatically upload your list or diagnostics. The **Diagnostics** button lets you save a local report. Reports can contain Steam IDs, IP addresses and paths: review them before sharing. Remove the settings folder if you also want to delete your saved data when uninstalling.

## Build from source

Use Python 3.9 or newer and the **32-bit MinGW-w64** GCC, G++ and `windres` tools. No game files are needed to build.

```sh
python build.py --package
```

The output folder and distributable ZIP are created in `build/`, named `UNI2-Block-List-0.1.0-rc.1-win32`. Omit `--package` to build only the folder. If the compiler tools are not on `PATH`, use `--cc`, `--cxx` and `--windres` to supply their paths.

## Disclaimer

This unofficial community tool is not affiliated with French-Bread, Arc System Works or Valve. It injects a DLL and hooks matchmaking functions in memory without modifying game files on disk. It is experimental and provided without warranty: use it at your own risk, respect the game's rules and expect that future updates may require changes. No game files are included.

## License

[MIT](LICENSE), © 2026 geturin. The bundled [MinHook](https://github.com/TsudaKageyu/minhook) library and its HDE components use the BSD 2-Clause license. Their original copyright notices and license terms are retained in [vendor/minhook/LICENSE.txt](vendor/minhook/LICENSE.txt) and included with the download.
