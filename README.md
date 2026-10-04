[English] | [Русский](READMEru.md)

# Warning, i didnt add english lang, so..

# Hinge Launcher

Cross-platform (Linux, Windows) Minecraft launcher written in C++20 with a
local web interface.

The launcher starts an HTTP server on localhost and opens the UI in your
browser — no extra software required. The whole frontend is plain
HTML/CSS/JS, embedded into the binary.

## Features

- **Official Microsoft authentication** — no proxies, no reuse of foreign
  client IDs. A fallback client ID is available if the primary one is
  unreachable.
- **Vanilla and modloaders** — Fabric, Quilt, Forge, NeoForge. One-click
  install, per-phase progress, raw installer output streamed to the
  built-in console.
- **Portable Java** — the launcher downloads the required JDK
  (Adoptium Temurin) on demand and injects it at launch. Required version
  is derived from the Minecraft version automatically.
- **Profiles** — offline and Microsoft, multiple `.minecraft` directories
  per profile, separate RAM allocation and Java path for each.
- **Built-in console** — real-time logs from the installer, modloader
  installers and Minecraft itself, filterable by level
  (INFO / WARN / ERROR).
- **No frontend build step** — the entire UI (HTML/CSS/JS) is embedded in
  `DefaultWeb.h` as raw string literals and unpacked into `web/` on first
  launch. No npm, webpack, or node_modules.

## Requirements

- CMake 3.16+
- A C++20-capable compiler (GCC 11+, Clang 14+, MSVC 19.30+)
- OpenSSL (dev package)

`httplib` and `nlohmann/json` are fetched automatically via `FetchContent`.

## Running

The server listens on `localhost:12345` and the UI opens in your browser.
The port can be changed in settings, or set to `-1` to pick a random free
port.

Data is stored next to the binary: `config.json`, `profiles/`,
`runtimes/`, `web/`.

## Editing the UI

Files in `web/` are read from disk on every request — edit the HTML, CSS,
or JS, save, and the tab reloads itself. If you delete a file, it will be
restored from the embedded copy on the next launch. Existing files are
never overwritten.

## Prebuilt binaries

Linux and Windows builds are available under
[Releases](https://github.com/Akfz/hinge/releases).

## License

MIT — see [LICENSE](LICENSE).
