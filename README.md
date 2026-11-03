# YMind - Mind Map Editor

[简体中文](README_zh.md) | English

A simple desktop mind map editor built with C++ and Qt6.

## Screenshots

![Screenshot](./img/AutoLayout.gif)
![Screenshot](./img/StartPage.png)
![Screenshot](./img/LightTheme.png)
![Screenshot](./img/DarkTheme.png)

## Features

- **Tabbed Editing** - Work on multiple mind maps simultaneously with drag-to-reorder tabs
- **Multiple Layouts** - Bilateral (balanced left/right), Top-Down (org chart), and Right-Tree (project plan) layouts
- **Auto Layout** - Automatically arrange nodes with `Ctrl+L`
- **Undo/Redo** - Full command-based undo/redo for add, remove, edit, and move operations
- **Drag & Drop** - Reposition nodes and subtrees by dragging
- **File I/O** - Save and load mind maps in `.ymind` (JSON) format
- **Export** - Export to PNG (2x scaling), SVG, PDF, plain text, or Markdown
- **Import** - Import mind maps from indented text files
- **Templates** - Start from built-in templates (Mind Map, Org Chart, Project Plan), load custom templates from JSON, or start with a blank canvas
- **Themes** - Light and Dark mode with system theme detection
- **Auto-Save** - Configurable automatic saving with 1-5 minute intervals
- **Outline Sidebar** - Tree-based outline view for quick navigation
- **Settings** - Configurable theme, fonts, auto-save, and editor preferences
- **Zoom & Pan** - Scroll wheel zoom, fit-to-view, and middle/right-click panning
- **Keyboard-Driven** - Comprehensive keyboard shortcuts for efficient editing
- **AI / MCP** - Ships with `ymind-cli`, a headless binary that doubles as a Model Context Protocol server. Lets Claude / Cursor / any MCP-compatible AI turn Markdown into a beautiful auto-laid-out mindmap. See the [setup guide](https://broccoli-97.github.io/xmind/#mcp).

## Headless rendering (CLI + MCP server)

Alongside the GUI, the build produces `ymind-cli` — a small companion binary for scripting and AI integration:

```bash
# Render Markdown to SVG/PNG
./build/ymind-cli render -i notes.md -o map.svg
./build/ymind-cli render -i notes.md -o map.png -f png --layout righttree --theme builtin.nord

# Stdin → stdout
cat notes.md | ./build/ymind-cli render -i - -o - -f svg > map.svg

# Run as a Model Context Protocol server (stdio)
./build/ymind-cli mcp
```

### Install as a Claude Code plugin (recommended)

YMind ships as a Claude Code plugin — two commands and you're done:

```
/plugin marketplace add broccoli-97/xmind
/plugin install ymind
```

Then ensure `ymind-cli` is on your PATH (one-time):

```bash
# After cmake --build build
sudo ln -s "$PWD/build/ymind-cli" /usr/local/bin/ymind-cli
# or, for a temporary session
export PATH="$PWD/build:$PATH"
```

Ask Claude: *"summarise this article as a mindmap, use the nord theme, save it to ~/Desktop"* — and it will. Full configuration steps for Claude Desktop and Cursor are on the [docs site](https://broccoli-97.github.io/xmind/#mcp).

## Building

### Prerequisites

- C++17 compiler (GCC, Clang, or MSVC)
- CMake 3.16+
- Qt6 (Widgets, Svg, PrintSupport, Network, LinguistTools)

### Build Instructions

```bash
mkdir build && cd build
cmake ..
make
./ymind
```

### macOS

Release packages target **macOS 11 (Big Sur) or later**, using Qt **6.7.3**.
Choose `macos-arm64` for Apple Silicon or `macos-x64` for Intel. Open the DMG
and drag `YMind.app` into Applications, or extract the `.tar.gz` and move the
complete application bundle there. Finder can open `.ymind` documents directly.

Build and package on macOS with the matching Qt kit on your PATH:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_DEPLOYMENT_TARGET=11.0
cmake --build build --parallel
open build/YMind.app
bash packaging/macos/package.sh build dist
python3 packaging/macos/smoke_test.py dist/YMind.app
```

The package script deploys Qt, CLI/MCP, plugins and translations, signs the
bundle, and checks every bundled binary's deployment target and dependencies.
Newer Qt versions may require newer macOS; the check rejects bundles that
advertise a lower minimum than their dependencies actually support.

The default signature is ad-hoc. For a release that passes Gatekeeper without
a first-launch exception, set `MACOS_CODESIGN_IDENTITY` to your Developer ID
Application identity and `MACOS_NOTARY_PROFILE` to a `notarytool` Keychain
profile before packaging. See [macOS packaging details](packaging/macos/README.md).

The installed CLI is inside the bundle:

```bash
/Applications/YMind.app/Contents/MacOS/ymind-cli mcp
sudo ln -s /Applications/YMind.app/Contents/MacOS/ymind-cli /usr/local/bin/ymind-cli
```

For desktop MCP clients, use the absolute bundle path as `command` if their
environment does not include `/usr/local/bin`. User templates and themes on
macOS live in `~/Library/Application Support/YMind/templates/` and
`~/Library/Application Support/YMind/themes/`.

### Build Options

```bash
# Build with unit tests
cmake .. -DBUILD_TESTING=ON

# Build with clang-tidy static analysis
cmake .. -DENABLE_CLANG_TIDY=ON

# Run tests
ctest --output-on-failure
```

## Keyboard Shortcuts

On macOS, use **Command (⌘)** for shortcuts shown with `Ctrl`. Redo also uses
`⌘+Shift+Z`; deleting a node with the Mac's Backspace key uses `⌘+Backspace`.

| Shortcut            | Action               |
| ------------------- | -------------------- |
| `Enter`             | Add child node       |
| `Ctrl+Enter`        | Add sibling node     |
| `Del`               | Delete selected node |
| `F2` / Double-click | Edit node text       |
| `Ctrl+L`            | Auto layout          |
| `Ctrl+T`            | New tab              |
| `Ctrl+W`            | Close tab            |
| `Ctrl+N`            | New file             |
| `Ctrl+O`            | Open file            |
| `Ctrl+S`            | Save                 |
| `Ctrl+Shift+S`      | Save as              |
| `Ctrl+Z`            | Undo                 |
| `Ctrl+Y`            | Redo                 |
| `Ctrl++`            | Zoom in              |
| `Ctrl+-`            | Zoom out             |
| `Ctrl+0`            | Fit to view          |
| `Ctrl+,`            | Settings             |
| Scroll wheel        | Zoom                 |
| Middle/Right-drag   | Pan                  |

## Project Structure

```
ymind/
├── CMakeLists.txt
├── .clang-tidy             # clang-tidy configuration
├── LICENSE
├── README.md
├── resources/
│   ├── resources.qrc
│   └── theme.qss           # QSS stylesheet template
├── tests/                   # Qt Test unit tests
│   ├── CMakeLists.txt
│   ├── tst_TemplateDescriptor.cpp
│   ├── tst_LayoutStyle.cpp
│   ├── tst_TemplateRegistry.cpp
│   ├── tst_LayoutAlgorithmRegistry.cpp
│   ├── tst_AppSettings.cpp
│   └── tst_MindMapSceneSerialization.cpp
└── src/
    ├── main.cpp
    ├── core/                # Application infrastructure
    │   ├── MainWindow       # Main window, menus, toolbar, auto-save
    │   ├── FileManager      # File I/O, export, and import
    │   ├── Commands         # Undo/redo commands (add, remove, edit, move)
    │   ├── AppSettings      # Settings singleton (theme, auto-save, fonts)
    │   ├── SettingsDialog   # Settings dialog UI
    │   ├── TemplateDescriptor  # Template data structures and JSON serialization
    │   └── TemplateRegistry    # Template registration and lookup
    ├── scene/               # Graphics scene items
    │   ├── MindMapScene     # Scene managing nodes, edges, serialization
    │   ├── MindMapView      # View with zoom, pan, and grid background
    │   ├── NodeItem         # Node graphics item with floating shadow
    │   └── EdgeItem         # Curved bezier edge connector
    ├── layout/              # Auto-layout algorithms
    │   ├── ILayoutAlgorithm       # Abstract algorithm interface
    │   ├── LayoutAlgorithmBase    # Shared measure/place/refine phases
    │   ├── BilateralLayout        # Left/right balanced tree
    │   ├── TopDownLayout          # Top-down org chart
    │   ├── RightTreeLayout        # Right-expanding tree
    │   ├── LayoutAlgorithmRegistry  # Algorithm registry singleton
    │   ├── LayoutEngine           # Static layout facade
    │   └── LayoutStyle            # Enum and name conversions
    └── ui/                  # Widgets and theming
        ├── ThemeManager     # Theme color palette singleton
        ├── StyleSheetGenerator  # CSS stylesheet generation
        ├── TabManager       # Tab bar and content stack management
        ├── StartPage        # Template gallery start page
        ├── OutlineWidget    # Tree-based outline sidebar
        └── IconFactory      # SVG icon and preview generation
```

## License

This project is licensed under the Apache License 2.0. See [LICENSE](LICENSE) for details.
