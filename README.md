# YMind - Mind Map Editor

[中文文档](README_zh.md)

A desktop mind map editor built with C++ and Qt6. Create, edit, and organize hierarchical mind maps with an intuitive interface featuring tabbed editing, multiple layout styles, undo/redo, and theme support.

## Screenshots

![Screenshot](./img/StartPage.png)
![Screenshot](./img/LightTheme.png)
![Screenshot](./img/DarkTheme.png)
![Screenshot](./img/AutoLayout.gif)

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

## Building

### Prerequisites

- C++17 compiler (GCC, Clang, or MSVC)
- CMake 3.16+
- Qt6 (Widgets, Svg, PrintSupport)

### Build Instructions

```bash
mkdir build && cd build
cmake ..
make
./ymind
```

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
