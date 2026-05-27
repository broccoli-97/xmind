#pragma once

#include "scene/StyleProvider.h"

// Concrete StyleProvider that delegates to the TemplateRegistry / ThemeRegistry
// singletons. This is the production implementation; MainWindow / Renderer /
// tests register it as the default at startup.
class RegistryStyleProvider final : public StyleProvider {
public:
    static RegistryStyleProvider& instance();

    const TemplateDescriptor* templateById(const QString& id) const override;
    const ThemeDescriptor* themeById(const QString& id) const override;
    QString defaultThemeId() const override;
    QString builtinTemplateIdForLayoutStyle(int layoutStyle) const override;
};
