# XMind - Mind Map Editor

A desktop mind map editor built with C++ and Qt6. Create, edit, and organize hierarchical mind maps with an intuitive interface featuring tabbed editing, auto-layout, undo/redo, and theme support.

## Features

- **Tabbed Editing** - Work on multiple mind maps simultaneously in browser-style tabs
- **Auto Layout** - Automatically arrange nodes in a balanced tree layout
- **Undo/Redo** - Full undo/redo support for all editing operations
- **Drag & Drop** - Reposition nodes and subtrees by dragging
- **File I/O** - Save and load mind maps in JSON-based `.xmind` format
- **Export** - Export to plain text or Markdown
- **Import** - Import mind maps from indented text files
- **Templates** - Start from built-in templates (Mind Map, Org Chart, Project Plan)
- **Themes** - Light and Dark mode with VS Code-inspired dark styling
- **Auto-Save** - Configurable automatic saving
- **Outline Sidebar** - Tree-based outline view for quick navigation
- **Keyboard-Driven** - Full keyboard shortcut support for efficient editing

## Screenshots

The application features a dark theme with a tabbed interface, sidebar outline, and template gallery.

## Building

### Prerequisites

- C++17 compiler (GCC, Clang, or MSVC)
- CMake 3.16+
- Qt6 Widgets

### Build Instructions

```bash
mkdir build && cd build
cmake ..
make
```

### Static Build (Windows)

```bash
cmake .. -DBUILD_STATIC=ON
make
```

### Run

```bash
./xmind
```

## Keyboard Shortcuts

| Shortcut | Action |
|----------|--------|
| `Enter` | Add child node |
| `Ctrl+Enter` | Add sibling node |
| `Del` | Delete selected node |
| `F2` / Double-click | Edit node text |
| `Ctrl+L` | Auto layout |
| `Ctrl+T` | New tab |
| `Ctrl+W` | Close tab |
| `Ctrl+N` | New file |
| `Ctrl+O` | Open file |
| `Ctrl+S` | Save |
| `Ctrl+Shift+S` | Save as |
| `Ctrl+Z` | Undo |
| `Ctrl+Y` | Redo |
| `Ctrl++` | Zoom in |
| `Ctrl+-` | Zoom out |
| `Ctrl+0` | Fit to view |
| `Ctrl+,` | Settings |
| Scroll wheel | Zoom |
| Middle/Right-drag | Pan |

## Project Structure

```
xmind/
├── CMakeLists.txt
├── LICENSE
├── README.md
└── src/
    ├── main.cpp              # Application entry point
    ├── MainWindow.h/cpp      # Main window with tabbed UI, menus, toolbar, sidebar
    ├── MindMapScene.h/cpp    # Graphics scene managing nodes, edges, and layout
    ├── MindMapView.h/cpp     # Graphics view with zoom and pan
    ├── NodeItem.h/cpp        # Node graphics item with text, styling, and hierarchy
    ├── EdgeItem.h/cpp        # Curved edge connector between nodes
    ├── Commands.h/cpp        # Undo/redo commands (add, remove, edit, move)
    ├── AppSettings.h/cpp     # Settings singleton (theme, auto-save, fonts)
    └── SettingsDialog.h/cpp  # Settings dialog UI
```

## License

This project is licensed under the Apache License 2.0. See [LICENSE](LICENSE) for details.
