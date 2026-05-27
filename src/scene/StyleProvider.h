#pragma once

#include <QString>

class TemplateDescriptor;
class ThemeDescriptor;

// Read-only lookup interface for templates and themes. Lets MindMapScene
// resolve style descriptors without pulling in TemplateRegistry/ThemeRegistry
// directly. App code injects a registry-backed implementation at startup;
// tests can inject a fake.
class StyleProvider {
public:
    virtual ~StyleProvider() = default;

    virtual const TemplateDescriptor* templateById(const QString& id) const = 0;
    virtual const ThemeDescriptor* themeById(const QString& id) const = 0;
    virtual QString defaultThemeId() const = 0;

    // Used by the v1→v2 JSON migrator to derive a templateId from the old
    // layoutStyle enum. Lives here so the migrator doesn't have to reach
    // back into the registry singleton.
    virtual QString builtinTemplateIdForLayoutStyle(int layoutStyle) const = 0;
};
