#include "core/RegistryStyleProvider.h"
#include "core/TemplateRegistry.h"
#include "core/ThemeRegistry.h"

RegistryStyleProvider& RegistryStyleProvider::instance() {
    static RegistryStyleProvider s;
    return s;
}

const TemplateDescriptor* RegistryStyleProvider::templateById(const QString& id) const {
    return TemplateRegistry::instance().templateById(id);
}

const ThemeDescriptor* RegistryStyleProvider::themeById(const QString& id) const {
    return ThemeRegistry::instance().themeById(id);
}

QString RegistryStyleProvider::defaultThemeId() const {
    return ThemeRegistry::defaultThemeId();
}

QString RegistryStyleProvider::builtinTemplateIdForLayoutStyle(int layoutStyle) const {
    return TemplateRegistry::builtinIdForLayoutStyle(layoutStyle);
}
