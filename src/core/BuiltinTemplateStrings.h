#pragma once

// Translation hints for built-in template and theme strings.
//
// Built-in template + theme definitions live as JSON files under
// resources/templates/ and resources/themes/, so the strings inside them are
// no longer scanned by Qt's lupdate from C++ source. This header re-declares
// every translatable built-in string via QT_TRANSLATE_NOOP so lupdate keeps
// emitting them into the .ts catalog. Templates use the "TemplateRegistry"
// context; themes use "ThemeRegistry". At runtime each loader translates
// parsed strings via QCoreApplication::translate with its own context.
// Keep this file in sync with the JSON files.

#include <QtGlobal>

namespace BuiltinTemplateStrings {

// ---------------------------------------------------------------------------
// Templates (layout + starter content)
// ---------------------------------------------------------------------------

// Mind Map
[[maybe_unused]] static const char* const kMindMapName = QT_TRANSLATE_NOOP("TemplateRegistry", "Mind Map");
[[maybe_unused]] static const char* const kMindMapDesc = QT_TRANSLATE_NOOP("TemplateRegistry", "Central topic with bilateral branches");
[[maybe_unused]] static const char* const kMindMapRoot = QT_TRANSLATE_NOOP("TemplateRegistry", "Central Topic");
[[maybe_unused]] static const char* const kMindMapB1 = QT_TRANSLATE_NOOP("TemplateRegistry", "Branch 1");
[[maybe_unused]] static const char* const kMindMapB2 = QT_TRANSLATE_NOOP("TemplateRegistry", "Branch 2");
[[maybe_unused]] static const char* const kMindMapB3 = QT_TRANSLATE_NOOP("TemplateRegistry", "Branch 3");
[[maybe_unused]] static const char* const kMindMapB4 = QT_TRANSLATE_NOOP("TemplateRegistry", "Branch 4");

// Org Chart
[[maybe_unused]] static const char* const kOrgName = QT_TRANSLATE_NOOP("TemplateRegistry", "Org Chart");
[[maybe_unused]] static const char* const kOrgDesc = QT_TRANSLATE_NOOP("TemplateRegistry", "Top-down organizational chart");
[[maybe_unused]] static const char* const kOrgRoot = QT_TRANSLATE_NOOP("TemplateRegistry", "CEO");
[[maybe_unused]] static const char* const kOrgEng = QT_TRANSLATE_NOOP("TemplateRegistry", "Engineering");
[[maybe_unused]] static const char* const kOrgMkt = QT_TRANSLATE_NOOP("TemplateRegistry", "Marketing");
[[maybe_unused]] static const char* const kOrgSales = QT_TRANSLATE_NOOP("TemplateRegistry", "Sales");

// Project Plan
[[maybe_unused]] static const char* const kProjName = QT_TRANSLATE_NOOP("TemplateRegistry", "Project Plan");
[[maybe_unused]] static const char* const kProjDesc = QT_TRANSLATE_NOOP("TemplateRegistry", "Right-tree project plan with phases and tasks");
[[maybe_unused]] static const char* const kProjRoot = QT_TRANSLATE_NOOP("TemplateRegistry", "Project");
[[maybe_unused]] static const char* const kProjP1 = QT_TRANSLATE_NOOP("TemplateRegistry", "Phase 1");
[[maybe_unused]] static const char* const kProjP2 = QT_TRANSLATE_NOOP("TemplateRegistry", "Phase 2");
[[maybe_unused]] static const char* const kProjT11 = QT_TRANSLATE_NOOP("TemplateRegistry", "Task 1.1");
[[maybe_unused]] static const char* const kProjT12 = QT_TRANSLATE_NOOP("TemplateRegistry", "Task 1.2");
[[maybe_unused]] static const char* const kProjT21 = QT_TRANSLATE_NOOP("TemplateRegistry", "Task 2.1");
[[maybe_unused]] static const char* const kProjT22 = QT_TRANSLATE_NOOP("TemplateRegistry", "Task 2.2");

// Lined
[[maybe_unused]] static const char* const kLinedName = QT_TRANSLATE_NOOP("TemplateRegistry", "Lined");
[[maybe_unused]] static const char* const kLinedDesc = QT_TRANSLATE_NOOP("TemplateRegistry", "Text floats on continuous baseline-anchored colored lines");
[[maybe_unused]] static const char* const kLinedRoot = QT_TRANSLATE_NOOP("TemplateRegistry", "Mind Mapping");
[[maybe_unused]] static const char* const kLinedC1 = QT_TRANSLATE_NOOP("TemplateRegistry", "Why Mind Mapping?");
[[maybe_unused]] static const char* const kLinedC1a = QT_TRANSLATE_NOOP("TemplateRegistry", "Disrupting linear thinking");
[[maybe_unused]] static const char* const kLinedC1b = QT_TRANSLATE_NOOP("TemplateRegistry", "Allows easy reorganization");
[[maybe_unused]] static const char* const kLinedC1c = QT_TRANSLATE_NOOP("TemplateRegistry", "Helps us think \"radiantly\"");
[[maybe_unused]] static const char* const kLinedC2 = QT_TRANSLATE_NOOP("TemplateRegistry", "Main benefits");
[[maybe_unused]] static const char* const kLinedC2a = QT_TRANSLATE_NOOP("TemplateRegistry", "Enables creative thinking");
[[maybe_unused]] static const char* const kLinedC2b = QT_TRANSLATE_NOOP("TemplateRegistry", "Combats perfectionism");
[[maybe_unused]] static const char* const kLinedC2c = QT_TRANSLATE_NOOP("TemplateRegistry", "Iterative thinking");
[[maybe_unused]] static const char* const kLinedC3 = QT_TRANSLATE_NOOP("TemplateRegistry", "Visual thinking");
[[maybe_unused]] static const char* const kLinedC4 = QT_TRANSLATE_NOOP("TemplateRegistry", "Helpful for pre-writing");
[[maybe_unused]] static const char* const kLinedC4a = QT_TRANSLATE_NOOP("TemplateRegistry", "Escape the blinking cursor");
[[maybe_unused]] static const char* const kLinedC4b = QT_TRANSLATE_NOOP("TemplateRegistry", "Healthy ideation strategies");
[[maybe_unused]] static const char* const kLinedC5 = QT_TRANSLATE_NOOP("TemplateRegistry", "How to Mind Map");
[[maybe_unused]] static const char* const kLinedC5a = QT_TRANSLATE_NOOP("TemplateRegistry", "Begin with an idea");
[[maybe_unused]] static const char* const kLinedC5b = QT_TRANSLATE_NOOP("TemplateRegistry", "Build connections outward");
[[maybe_unused]] static const char* const kLinedC5c = QT_TRANSLATE_NOOP("TemplateRegistry", "Reorganize as needed");

// ---------------------------------------------------------------------------
// Themes (visual styling — no layout or content)
// ---------------------------------------------------------------------------

// Default
[[maybe_unused]] static const char* const kThemeDefaultName = QT_TRANSLATE_NOOP("ThemeRegistry", "Default");
[[maybe_unused]] static const char* const kThemeDefaultDesc = QT_TRANSLATE_NOOP("ThemeRegistry", "Classic look: solid colored fills with a soft drop shadow");

// Outlined
[[maybe_unused]] static const char* const kThemeOutlinedName = QT_TRANSLATE_NOOP("ThemeRegistry", "Outlined");
[[maybe_unused]] static const char* const kThemeOutlinedDesc = QT_TRANSLATE_NOOP("ThemeRegistry", "Minimalist style — colored borders, no fills, no shadow");

// Tinted
[[maybe_unused]] static const char* const kThemeTintedName = QT_TRANSLATE_NOOP("ThemeRegistry", "Tinted");
[[maybe_unused]] static const char* const kThemeTintedDesc = QT_TRANSLATE_NOOP("ThemeRegistry", "Soft tinted fills with colored borders — Excalidraw-style");

// Morandi
[[maybe_unused]] static const char* const kThemeMorandiName = QT_TRANSLATE_NOOP("ThemeRegistry", "Morandi");
[[maybe_unused]] static const char* const kThemeMorandiDesc = QT_TRANSLATE_NOOP("ThemeRegistry", "Refined Morandi palette — soft, painterly hues with a clean modern lift");

// Nord
[[maybe_unused]] static const char* const kThemeNordName = QT_TRANSLATE_NOOP("ThemeRegistry", "Nord");
[[maybe_unused]] static const char* const kThemeNordDesc = QT_TRANSLATE_NOOP("ThemeRegistry", "Frosted Nordic palette — cool blues, sage, and soft warm accents");

// Sakura
[[maybe_unused]] static const char* const kThemeSakuraName = QT_TRANSLATE_NOOP("ThemeRegistry", "Sakura");
[[maybe_unused]] static const char* const kThemeSakuraDesc = QT_TRANSLATE_NOOP("ThemeRegistry", "Cherry-blossom palette — soft pinks, fresh leaf greens, and wisteria");

// Forest
[[maybe_unused]] static const char* const kThemeForestName = QT_TRANSLATE_NOOP("ThemeRegistry", "Forest");
[[maybe_unused]] static const char* const kThemeForestDesc = QT_TRANSLATE_NOOP("ThemeRegistry", "Botanical journal palette — mossy greens, bark, and autumn warmth on paper");

// Candy
[[maybe_unused]] static const char* const kThemeCandyName = QT_TRANSLATE_NOOP("ThemeRegistry", "Candy");
[[maybe_unused]] static const char* const kThemeCandyDesc = QT_TRANSLATE_NOOP("ThemeRegistry", "Playful pastel palette — macaron pinks, mints, peach, and buttercup");

// Sketch
[[maybe_unused]] static const char* const kThemeSketchName = QT_TRANSLATE_NOOP("ThemeRegistry", "Sketch");
[[maybe_unused]] static const char* const kThemeSketchDesc = QT_TRANSLATE_NOOP("ThemeRegistry", "Hand-drawn marker on warm paper — Excalidraw-style wobbly outlines");

// Whimsy
[[maybe_unused]] static const char* const kThemeWhimsyName = QT_TRANSLATE_NOOP("ThemeRegistry", "Whimsy");
[[maybe_unused]] static const char* const kThemeWhimsyDesc = QT_TRANSLATE_NOOP("ThemeRegistry", "Sunset confetti — saturated solid pills on warm peach");

// Sequoia
[[maybe_unused]] static const char* const kThemeSequoiaName = QT_TRANSLATE_NOOP("ThemeRegistry", "Sequoia");
[[maybe_unused]] static const char* const kThemeSequoiaDesc = QT_TRANSLATE_NOOP("ThemeRegistry", "macOS-style pills — system colors per branch on a neutral canvas");

// Cupertino
[[maybe_unused]] static const char* const kThemeCupertinoName = QT_TRANSLATE_NOOP("ThemeRegistry", "Cupertino");
[[maybe_unused]] static const char* const kThemeCupertinoDesc = QT_TRANSLATE_NOOP("ThemeRegistry", "White cards with branch accent bars — macOS notification style");

// Frost
[[maybe_unused]] static const char* const kThemeFrostName = QT_TRANSLATE_NOOP("ThemeRegistry", "Frost");
[[maybe_unused]] static const char* const kThemeFrostDesc = QT_TRANSLATE_NOOP("ThemeRegistry", "Frosted glass tints on a cool gray canvas — macOS materials");

} // namespace BuiltinTemplateStrings
