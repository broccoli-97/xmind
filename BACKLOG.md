# YMind Backlog

Features, bugs, and refactors identified during a multi-perspective review (PM / UI eng / IxD / architect).
Check items off as `[x]` when done. File paths and line numbers are anchors at the time of writing — verify before working.

Legend: **P0** must-fix / table-stakes · **P1** big win · **P2** nice-to-have · **P3** speculative
Effort: **S** <1 day · **M** 1–3 days · **L** >3 days

---

## Features

### Editing & navigation
- [x] **P0 / M** Subtree collapse/expand. Done: `NodeItem::setCollapsed/toggleCollapsed/isCollapsed` hides descendants + incident edges recursively (nested collapses survive an ancestor expand). Painted "···" indicator inside the right edge of collapsed nodes that have children. Space toggles collapse on the selected non-root node. Arrow keys refuse to land on hidden descendants. JSON format bumped v3→v4 with a per-node `"collapsed": true` field (omitted when false; v3 files read back as all-expanded via the no-op migrator). Tier-3 coverage in `tests/tst_NodeCollapse.cpp` (7 cases).
- [x] **P0 / M** In-map search (`Ctrl+F`). Done: pure `MindMapScene::findMatches(needle, cs)` returns matches in pre-order; new `ui/FindBar` widget (inline bar above the canvas) emits queryChanged/stepNext/stepPrev/closed; `MainWindow` drives highlights via `NodeItem::setSearchMatch/setSearchCurrent` (yellow tint, brighter ring for current) and scrolls each match into view. F3/Shift+F3 step, Esc closes, Ctrl+F (re-)activates. Tier-3 coverage in `tests/tst_SceneFindMatches.cpp` (7 cases: empty/case-sensitivity/pre-order/substring/no-match/clear).
- [x] **P0 / S** Arrow-key navigation between nodes. Done: `MindMapScene::findNeighbor(node, NavDirection)` returns the next node for ←↑↓→/Tab/Shift+Tab, layout-aware (siblings stack on the perpendicular axis; "deeper" direction mirrors per-side for bilateral). `keyPressEvent` wires it up and scrolls the target into view. Tier-3 coverage in `tests/tst_MindMapSceneNav.cpp` (12 cases across all three layouts).
- [ ] **P0 / M** Cut / copy / paste of nodes and subtrees. _Deferred per user direction during the bug-fix pass._
- [ ] **P1 / M** Drag-to-reparent. _Deferred per user direction during this pass._
- [ ] **P1 / S** Multi-select (rubber-band + Ctrl-click). _Deferred per user direction during this pass._
- [ ] **P1 / S** Insert-parent (wrap selection in a new parent). _Deferred per user direction during this pass._
- [ ] **P1 / S** Keyboard sibling reorder (Alt+↑/↓). _Deferred per user direction during this pass._
- [ ] **P2 / S** Per-node context menu (right-click): rename, duplicate, copy-as-image, move-to-tab, color override.
- [ ] **P2 / M** Focus / zen mode — dim everything outside the selected subtree; `Ctrl+.` fit-to-subtree.
- [ ] **P2 / L** Presentation mode (walk nodes one branch at a time).

### Node content
- [ ] **P1 / L** Rich node content: notes, images, hyperlinks, attachments, code blocks, icons, priority/progress markers, tags. `NodeItem` carries only `QString m_text` today. _Deferred per user direction during this pass (too large for one session)._
- [ ] **P2 / S** Per-node color override (currently colors derive only from level/branch palette).
- [ ] **P3 / L** Cross-link / relationship edges (non-tree edges).

### Import / export
- [ ] **P1 / S** OPML import — common interop format, cheap to add next to the Markdown importer. _Deferred per user direction during this pass._
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

- [x] **P0** Untitled tabs have **no auto-save / crash recovery**. Done: new `RecoveryManager` (per-launch UUID dirs under `AppLocalDataLocation/recovery/<uuid>`) writes atomic snapshots via `QSaveFile` on every auto-save tick for untitled-modified tabs. `MainWindow` detects orphan sessions on startup and prompts the user to restore; graceful exit clears the current session dir. Tier-2 coverage in `tests/tst_RecoveryManager.cpp`.
- [x] **P0** `Backspace` deletes the selected node outside of edit mode. Done: bare Backspace now passes through; only `Delete` and `Ctrl/Cmd+Backspace` delete (`MindMapScene.cpp::keyPressEvent`). Covered by `tests/tst_MindMapSceneKeys.cpp`.
- [ ] **P0** Right-click drag is repurposed for panning (`src/scene/MindMapView.cpp:50`), silently consuming what should be the context-menu gesture. There is no per-node context menu at all. Replace with space-drag or middle-only.
- [x] **P1** File-format version drift in docs. Done: the local-only `CLAUDE.md` (which is gitignored) now matches `kCurrentVersion = 3` and points at the migrator table. The original BACKLOG note also blamed `README.md`, but `README.md` never actually mentioned a version — no fix needed there.
- [x] **P1** Auto-layout always zooms-to-fit on `Ctrl+L`. Done: `autoLayout`'s finish callback now calls the new `MindMapView::zoomToFitIfOffscreen()`, backed by pure predicate `rectFullyVisibleIn` (Tier-1 tested in `tst_ViewGeometry.cpp`). A deliberate zoom-in is preserved when content remains visible.
- [x] **P1** Auto-save interval capped at **5 minutes**. **WontFix** — user confirmed 5 min is the intended ceiling.
- [x] **P1** Outline panel does not appear to sync to scene-selection changes live. **Stale note** — `OutlineWidget::refresh` already wires `selectionChanged → syncSelection`, and `MainWindow` calls `refresh` on every tab change. Added Tier-3 regression coverage in `tests/tst_OutlineWidget.cpp` to keep it that way.
- [x] **P2** Scene rect is hard-coded `(-5000,-5000,10000,10000)`. Done: the initial rect stays as a floor (so an empty scene keeps pan headroom), and a new `MindMapView::recomputeSceneRect` unions it with `itemsBoundingRect() + 2000px margin`. Debounced by a 200ms timer connected to `QGraphicsScene::changed`. Verified manually — dragging a node past 5000px now grows scrollbars cleanly.
- [x] **P2** Hover-add button briefly raises node `zValue` to 50. Done: bumped from 50 down to `kHoverZ = 2.0` (enough to top sibling nodes at z=0; well under the inline editor at z=100). `showAddButton` now guards `m_savedZValue` against a rapid leave/re-enter overwriting the saved baseline. `InlineEditController::startEditing` invokes a new `NodeItem::cancelAddButton()` so the overlay is torn down immediately when an editor opens, rather than visibly hanging during its fade-out.
- [x] **P2** No visible "press Esc to cancel" affordance during inline edit. Done as part of the contextual-status-bar fix below.
- [x] **P2** Status-bar hint text is static. Done: new `MainWindow::updateStatusHint` swaps between three messages — start-page text, "Enter: Commit | Esc: Cancel" during inline edit (driven by new `InlineEditController::editingStarted/editingFinished` signals re-emitted from `MindMapScene`), and the existing idle shortcut hint. Tier-3 `tests/tst_InlineEditController.cpp` covers the signal plumbing.
- [x] **P1** Auto-layout trigger rules clarified and enforced. Full re-layout now runs only on explicit gestures: Ctrl+L/toolbar (fit-if-offscreen, unchanged), template switch (now fits unconditionally — the old `zoomToFitIfOffscreen` left compact layouts badly framed — and now dirties the document via `markModified` in `setTemplateId`/`setThemeId`, which previously lost the switch silently on close), and initial content (new-from-template, imports). Confirming a node text edit no longer re-layouts the map (`EditTextCommand` dropped its `autoLayout()` calls — a user zoomed into a large map stays put; same for undo/redo of edits). Window-resize refit keeps node positions and now reserves breathing room of one node's width (`MindMapView::fitMarginForNodeWidth`, clamped 80–400px, replacing the fixed 80px). Contract documented on `MindMapScene::autoLayout()`; covered by Tier-3 `tests/tst_AutoLayoutTriggers.cpp` and Tier-1 margin cases in `tst_ViewGeometry.cpp`.
- [x] **P2** Layout algorithms ignore collapse state. Done: `measureSubtree`/`placeSubtree` treat a collapsed node as a leaf (its branch stops reserving spread), `collectAllNodes` skips hidden subtrees during overlap-avoidance placement, and `LayoutEngine::computeLayout` stacks hidden descendants onto their folded ancestor's final position so invisible strays can't inflate `itemsBoundingRect` (zoom-to-fit/exports) and expansion unfolds outward from the parent. Collapse/expand is now an explicit re-layout gesture: `MindMapScene::toggleNodeCollapsed` (Space, the new canvas fold controls, outline folds all route through it) flips state, dirties the document, emits `nodeCollapseChanged`, and runs the animated auto-layout; `addChildToSelected` auto-expands a folded target first. The old non-clickable "..." indicator is replaced by a macOS-style fold UI: a persistent count badge (branch-colored pill, hidden-topic count, click-to-expand on mouse-up without changing selection, hover highlight + pointing-hand cursor) at the child-side junction while folded, and a hover-revealed collapse chevron next to the (outward-shifted) add "+" in `AddButtonOverlay` while expanded; the hover "+" is suppressed while folded. Covered in `tests/tst_NodeCollapse.cpp` (9 new cases).

---

## Refactors / tech debt

- [x] **P1 / M** Split `MainWindow.cpp` (~~968 lines~~ → 571). Extracted `AutoSaveManager` (`src/core/AutoSaveManager.{h,cpp}`), `UpdateNotifier` (`src/core/UpdateNotifier.{h,cpp}`), `MindMapToolBar` (`src/ui/MindMapToolBar.{h,cpp}`), and `TemplateMenuController` / `ThemeMenuController` (`src/ui/`). File/Edit/View menu construction stays in `MainWindow::setupMenuBar` — too tightly coupled to the rest to extract cleanly.
- [x] **P1 / M** Split `NodeItem.cpp`. ~~`effStyle/effFont/effPadding/...` helpers → `NodeStyleResolver`~~ done. `AddButtonOverlay` extracted to `src/scene/AddButtonOverlay.{h,cpp}` (NodeItem.cpp is now 593 lines).
- [x] **P1 / M** Replace triple cache-thrash on theme/template swap. ~~Today `MainWindow::applyThemeId` (lines 798-815), `applyTemplateId` (lines 728-746) **and** `MindMapScene::setThemeId` (lines 185-196) all walk every scene item to drop and restore cache.~~ Done: extracted `MindMapScene::invalidateStyle()` (private); `setThemeId`/`setTemplateId` now own the dance. MainWindow `applyThemeId`/`applyTemplateId` reduced to scene-setter calls and dropped `NodeItem`/`QGraphicsItem`/`QPointer` includes.
- [x] **P1 / S** `MindMapScene::removeNode` is O(n²) on large subtrees — ~~walks `m_edges` linearly per child~~. Done: now uses each `NodeItem`'s own `edges()` list (`MindMapScene.cpp:89-122`).
- [x] **P1 / S** Helper to kill repetition: dozens of `[this](){ if (auto* s = m_tabManager->currentScene()) s->...; }` lambdas in `MainWindow.cpp`. Done: `withCurrentScene(F&&)` / `withCurrentView(F&&)` template helpers added on `MainWindow`; menu+action wiring uses them throughout.
- [ ] **P1 / L** Document/model separation. `MindMapScene` is canvas + model. Introduce `MindMapDocument` (pure data + commands); GUI scene wraps one; `ymind-cli` consumes one directly so it can link only QtCore/QtGui (not QtWidgets). **Not done.** Requires pulling node/edge data out of `NodeItem`/`EdgeItem` (currently `QGraphicsObject`) into pure-data types — multi-session refactor that ripples through `MindMapSerializer`, `MindMapExporter`, `LayoutEngine`, and `Commands`.
- [x] **P1 / M** `scene/` should not import `core/templates` and `core/themes` directly. Done: `scene/StyleProvider.h` defines a read-only lookup interface; `core/RegistryStyleProvider` wraps the singletons. `MindMapScene` calls through the provider — registered globally via `MindMapScene::setDefaultStyleProvider`. Scene/ no longer includes `TemplateRegistry.h` or `ThemeRegistry.h` anywhere.
- [x] **P1 / S** Make layout selection explicit on the document. ~~Today `MindMapScene::setTemplateId` (lines 156-167) silently rewrites layout style as a hidden side effect.~~ Done by a simpler route than the planned enum: `layoutStyle()` now *derives* from the active template at read-time; `m_layoutStyle` becomes the fallback for template-less scenes (legacy v1 files, CLI without `--template`). `setTemplateId` no longer mutates the field; `layoutStyleChanged` fires only when the *effective* value changes.
- [x] **P1 / L** Versioned JSON migrators: `migrators[fromVersion](json) -> json` chained until current. Done: `MindMapSerializer::migrate` walks a static `migrators` table (`v1→v2`, `v2→v3`) until the doc reaches `kCurrentVersion`. Adding a new format bump is appending a function + bumping the constant.
- [x] **P2 / M** Replace ad-hoc singletons with a `Services` aggregate. Done: `core/Services.{h,cpp}` bundles pointers to `AppSettings`, `TemplateRegistry`, `ThemeRegistry`, `LayoutAlgorithmRegistry`, and `StyleProvider`. `MainWindow` constructor now takes `const Services&`; `main.cpp` builds one via `Services::productionDefaults()`. `TabManager`/`FileManager` don't yet take a Services because neither uses any of the singletons directly — when they need to, pass one in.
- [x] **P2 / S** Strong-typedef `TemplateId` / `ThemeId`. Done: `core/Identifiers.h` defines tag-templated wrappers around `QString` with `isValid()` / `toString()` / `operator==`. `MindMapScene`'s API, the menu controllers, and the serializer use them; the scattered `if (!id.isEmpty())` checks are gone.

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
