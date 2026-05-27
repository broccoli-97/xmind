#include "core/Services.h"
#include "core/AppSettings.h"
#include "core/RegistryStyleProvider.h"
#include "core/TemplateRegistry.h"
#include "core/ThemeRegistry.h"
#include "layout/LayoutAlgorithmRegistry.h"

Services Services::productionDefaults() {
    return Services{
        /*settings=*/&AppSettings::instance(),
        /*templates=*/&TemplateRegistry::instance(),
        /*themes=*/&ThemeRegistry::instance(),
        /*layouts=*/&LayoutAlgorithmRegistry::instance(),
        /*styleProvider=*/&RegistryStyleProvider::instance(),
    };
}
