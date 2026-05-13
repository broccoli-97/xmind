#pragma once

// Translation hints for built-in template strings.
//
// Built-in template definitions live as JSON files under resources/templates/,
// so the strings inside them are no longer scanned by Qt's lupdate from C++
// source. This header re-declares every translatable built-in string via
// QT_TRANSLATE_NOOP("TemplateRegistry", ...) so lupdate keeps emitting them
// into the .ts catalog under the same context name as before.
//
// At runtime, the loader translates each parsed string via
// QCoreApplication::translate("TemplateRegistry", <utf8>). Keep this file in
// sync when you add/rename/remove strings in resources/templates/*.json.

#include <QtGlobal>

namespace BuiltinTemplateStrings {

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
[[maybe_unused]] static const char* const kLinedDesc = QT_TRANSLATE_NOOP("TemplateRegistry", "Curved colored lines, text floats above the line");
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

// Outlined
[[maybe_unused]] static const char* const kOutlinedName = QT_TRANSLATE_NOOP("TemplateRegistry", "Outlined");
[[maybe_unused]] static const char* const kOutlinedDesc = QT_TRANSLATE_NOOP("TemplateRegistry", "Minimalist style — colored borders, no fills, no shadow");

// Tinted
[[maybe_unused]] static const char* const kTintedName = QT_TRANSLATE_NOOP("TemplateRegistry", "Tinted");
[[maybe_unused]] static const char* const kTintedDesc = QT_TRANSLATE_NOOP("TemplateRegistry", "Soft tinted fills with colored borders — Excalidraw-style");

} // namespace BuiltinTemplateStrings
