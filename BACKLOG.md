# YMind Backlog

Features, bugs, and refactors identified during a multi-perspective review (PM / UI eng / IxD / architect).
Check items off as `[x]` when done. File paths and line numbers are anchors at the time of writing — verify before working.

Legend: **P0** must-fix / table-stakes · **P1** big win · **P2** nice-to-have · **P3** speculative
Effort: **S** <1 day · **M** 1–3 days · **L** >3 days

---

## Features

### Editing & navigation
- [ ] **P0 / M** Subtree collapse/expand. Add fold state to `NodeItem`, hide collapsed descendants and their edges, show a chevron on hover. Persist in JSON (bump format).
- [ ] **P0 / M** In-map search (`Ctrl+F`). Inline find bar; highlight matches in scene + outline; next/prev nav. No `find` exists in `MindMapScene` today.
- [ ] **P0 / S** Arrow-key navigation between nodes (Tab/Shift+Tab to siblings, arrows to walk the tree by layout orientation). Currently selection only changes via mouse — accessibility blocker.
- [ ] **P0 / M** Cut / copy / paste of nodes and subtrees. JSON serializer already round-trips — reuse it for clipboard MIME `application/x-ymind-subtree`.
- [ ] **P1 / M** Drag-to-reparent. `NodeItem::mouseMoveEvent` currently moves the subtree by delta but never reparents on drop. Add hit-test + `ReparentNodeCommand`.
- [ ] **P1 / S** Multi-select (rubber-band + Ctrl-click). Commands must accept lists; today `selectedNode()` returns the first match.
- [ ] **P1 / S** Insert-parent (wrap selection in a new parent).
- [ ] **P1 / S** Keyboard sibling reorder (Alt+↑/↓).
- [ ] **P2 / S** Per-node context menu (right-click): rename, duplicate, copy-as-image, move-to-tab, color override.
- [ ] **P2 / M** Focus / zen mode — dim everything outside the selected subtree; `Ctrl+.` fit-to-subtree.
- [ ] **P2 / L** Presentation mode (walk nodes one branch at a time).

### Node content
- [ ] **P1 / L** Rich node content: notes, images, hyperlinks, attachments, code blocks, icons, priority/progress markers, tags. `NodeItem` carries only `QString m_text` today.
- [ ] **P2 / S** Per-node color override (currently colors derive only from level/branch palette).
- [ ] **P3 / L** Cross-link / relationship edges (non-tree edges).

### Import / export
- [ ] **P1 / S** OPML import — common interop format, cheap to add next to the Markdown importer.
- [ ] **P2 / M** FreeMind `.mm` import.
- [ ] **P2 / L** XMind `.xmind` import.
- [ ] **P2 / S** Paste-clipboard-text → new map.

### Layouts
- [ ] **P2 / M** Fishbone layout.
- [ ] **P2 / M** Timeline layout.
- [ ] **P2 / M** Radial layout.
- [ ] **P3 / L** Tree-map / matrix layouts.

### Workspace
- [ ] **P1 / S** Recent files list (start page + File menu). Not currently exposed.
- [ ] **P1 / S** Per-tab persisted zoom and pan (every reload re-fits).
- [ ] **P2 / S** Mini-map widget in the bottom-right of the view (scene rect is 10000×10000 — easy to get lost).
- [ ] **P2 / M** Plugin / extension surface: custom layout `.so/.dll` symbol, MCP write API for AI agents to *edit* (not just generate) maps.

---

## Bugs

- [ ] **P0** Untitled tabs have **no auto-save / crash recovery**. `MainWindow::onAutoSaveTimeout` (`src/core/MainWindow.cpp:851`) requires `!tab.filePath.isEmpty()` and skips. Need a scratch in `$XDG_DATA_HOME/YMind/recovery/`.
- [ ] **P0** `Backspace` deletes the selected node outside of edit mode (`src/scene/MindMapScene.cpp:287`). Hostile when users tap it expecting to correct a typo. Limit to `Delete` (or require `Ctrl/Cmd+Backspace`).
- [ ] **P0** Right-click drag is repurposed for panning (`src/scene/MindMapView.cpp:50`), silently consuming what should be the context-menu gesture. There is no per-node context menu at all. Replace with space-drag or middle-only.
- [ ] **P1** File-format version drift: `MindMapSerializer.cpp:31` writes `version = 3`, but `README.md` and `CLAUDE.md` both say version 2. No documented migrator either.
- [ ] **P1** Auto-layout always zooms-to-fit on `Ctrl+L` (`MindMapScene.cpp:459`), yanking the user out of any deliberate zoom. Only fit if content is currently off-screen.
- [ ] **P1** Auto-save interval capped at **5 minutes** (`SettingsDialog.cpp:56`, `setRange(1, 5)`) — arbitrary low ceiling.
- [ ] **P1** Outline panel does not appear to sync to scene-selection changes live. `MainWindow.cpp:90-93` only wires the undo stack to `refreshOutline`. Wire `QGraphicsScene::selectionChanged` → outline highlight.
- [ ] **P2** Scene rect is hard-coded `(-5000,-5000,10000,10000)` in `MindMapView.cpp:26`. Large maps will scroll off the world. Make it grow with `itemsBoundingRect()`.
- [ ] **P2** Hover-add button briefly raises node `zValue` to 50; can flicker over the clicked overlay button (`NodeItem.cpp:640`).
- [ ] **P2** No visible "press Esc to cancel" affordance during inline edit.
- [ ] **P2** Status-bar hint text is static (`MainWindow.cpp:907-911`) — long shortcut soup. Make contextual to selection / edit state.

---

## Refactors / tech debt

- [ ] **P1 / M** Split `MainWindow.cpp` (968 lines). Extract `MenuBarBuilder`, `AutoSaveManager`, `UpdateNotifier`, `ToolBarBuilder`. Resists testing today (no `tst_MainWindow`).
- [ ] **P1 / M** Split `NodeItem.cpp` (845 lines). `AddButtonOverlay` → its own file; `effStyle/effFont/effPadding/...` helpers → `NodeStyleResolver` class with memoization by `(theme, template, level)`. `boundingRect`/`shape`/`paint`/`updateGeometry` all recompute these.
- [x] **P1 / M** Replace triple cache-thrash on theme/template swap. ~~Today `MainWindow::applyThemeId` (lines 798-815), `applyTemplateId` (lines 728-746) **and** `MindMapScene::setThemeId` (lines 185-196) all walk every scene item to drop and restore cache.~~ Done: extracted `MindMapScene::invalidateStyle()` (private); `setThemeId`/`setTemplateId` now own the dance. MainWindow `applyThemeId`/`applyTemplateId` reduced to scene-setter calls and dropped `NodeItem`/`QGraphicsItem`/`QPointer` includes.
- [ ] **P1 / S** `MindMapScene::removeNode` is O(n²) on large subtrees — walks `m_edges` linearly per child (`MindMapScene.cpp:99-110`). Each `NodeItem` already tracks its own `m_edges`; use that.
- [ ] **P1 / S** Helper to kill repetition: dozens of `[this](){ if (auto* s = m_tabManager->currentScene()) s->...; }` lambdas in `MainWindow.cpp:351-541`. Replace with `withCurrentScene(F&&)` / `withCurrentView(F&&)`.
- [ ] **P1 / L** Document/model separation. `MindMapScene` is canvas + model. Introduce `MindMapDocument` (pure data + commands); GUI scene wraps one; `ymind-cli` consumes one directly so it can link only QtCore/QtGui (not QtWidgets).
- [ ] **P1 / M** `scene/` should not import `core/templates` and `core/themes` directly (`MindMapScene.cpp:3-5`). Invert with a `StyleProvider` interface so headless/CLI renderer doesn't drag in registries.
- [x] **P1 / S** Make layout selection explicit on the document. ~~Today `MindMapScene::setTemplateId` (lines 156-167) silently rewrites layout style as a hidden side effect.~~ Done by a simpler route than the planned enum: `layoutStyle()` now *derives* from the active template at read-time; `m_layoutStyle` becomes the fallback for template-less scenes (legacy v1 files, CLI without `--template`). `setTemplateId` no longer mutates the field; `layoutStyleChanged` fires only when the *effective* value changes.
- [ ] **P1 / L** Versioned JSON migrators: `migrators[fromVersion](json) -> json` chained until current. Required before any further format bumps.
- [ ] **P2 / M** Replace ad-hoc singletons (`AppSettings`, `TemplateRegistry`, `ThemeRegistry`, `LayoutAlgorithmRegistry`, `ThemeManager`) with a `Services` aggregate passed into `MainWindow`/`TabManager`/`FileManager`. Lets tests substitute fakes; lets a second window type reuse construction.
- [ ] **P2 / S** Strong-typedef `TemplateId` / `ThemeId` to eliminate scattered `if (!id.isEmpty())` defensive checks.

---

## Testing gaps

- [ ] **P0 / M** Commands undo/redo round-trip tests, especially `RemoveNodeCommand` snapshot. Currently only one Tier-3 test (`tst_MindMapSceneSerialization.cpp`).
- [ ] **P1 / M** `InlineEditController` state-machine tests.
- [ ] **P1 / M** Edge-path stability after node geometry changes.
- [ ] **P1 / L** Golden-image render tests in `tests/golden_render/`. Render a fixed map per theme to PNG, byte-diff vs. checked-in reference. Would have caught recent theme-refactor regressions visible in `git log`.
- [ ] **P2 / S** Fuzz the Markdown importer (strict mode exists, good — just feed it garbage).

---

## Accessibility

- [ ] **P0 / M** Zero `setAccessibleName`/`setAccessibleDescription` calls in the codebase. Retrofit toolbar buttons, menu actions, dialogs first; then nodes via `QAccessibleInterface`.
- [ ] **P0** Arrow-key node navigation (also listed under Features) — required for keyboard-only users.
- [ ] **P2 / M** High-contrast theme variant; respect OS reduce-motion (currently animations are always on: `MindMapView.cpp:149-169`, `MindMapScene.cpp:447-462`).
