#pragma once

class AppSettings;
class LayoutAlgorithmRegistry;
class StyleProvider;
class TemplateRegistry;
class ThemeRegistry;

// Bundles the long-lived application-wide collaborators (settings, registries,
// style provider) into a single value passed into MainWindow / TabManager /
// FileManager. Replaces the ad-hoc `X::instance()` calls scattered through the
// app: now construction takes its dependencies explicitly, tests can substitute
// fakes by building a Services with their own pointers, and a second window
// type (e.g. a secondary editor) can share one.
//
// Holds non-owning pointers — the production singletons remain the owners.
struct Services {
    AppSettings* settings = nullptr;
    TemplateRegistry* templates = nullptr;
    ThemeRegistry* themes = nullptr;
    LayoutAlgorithmRegistry* layouts = nullptr;
    StyleProvider* styleProvider = nullptr;

    // Convenience factory: wraps the production singletons. Call this in
    // main.cpp; tests can build their own Services with mocks instead.
    static Services productionDefaults();
};
