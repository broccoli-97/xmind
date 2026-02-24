#include "AppSettings.h"

#include <QSettings>
#include <algorithm>

AppSettings& AppSettings::instance() {
    static AppSettings s;
    return s;
}

AppSettings::AppSettings() : QObject(nullptr) {
    m_settings = new QSettings(this);
}

AppSettings::~AppSettings() = default;

AppTheme AppSettings::theme() const {
    int val = m_settings->value("appearance/theme", 1).toInt();
    return (val == 1) ? AppTheme::Dark : AppTheme::Light;
}

void AppSettings::setTheme(AppTheme theme) {
    int old = m_settings->value("appearance/theme", 1).toInt();
    int val = (theme == AppTheme::Dark) ? 1 : 0;
    if (old != val) {
        m_settings->setValue("appearance/theme", val);
        emit themeChanged(theme);
    }
}

bool AppSettings::autoSaveEnabled() const {
    return m_settings->value("autosave/enabled", false).toBool();
}

void AppSettings::setAutoSaveEnabled(bool enabled) {
    m_settings->setValue("autosave/enabled", enabled);
    emit autoSaveSettingsChanged();
}

int AppSettings::autoSaveIntervalMinutes() const {
    int val = m_settings->value("autosave/interval", 2).toInt();
    return std::clamp(val, 1, 5);
}

void AppSettings::setAutoSaveIntervalMinutes(int minutes) {
    minutes = std::clamp(minutes, 1, 5);
    m_settings->setValue("autosave/interval", minutes);
    emit autoSaveSettingsChanged();
}

int AppSettings::defaultFontSize() const {
    int val = m_settings->value("editor/defaultFontSize", 12).toInt();
    return std::clamp(val, 8, 24);
}

void AppSettings::setDefaultFontSize(int size) {
    size = std::clamp(size, 8, 24);
    int old = m_settings->value("editor/defaultFontSize", 12).toInt();
    if (old != size) {
        m_settings->setValue("editor/defaultFontSize", size);
        emit defaultFontSizeChanged(size);
    }
}

QByteArray AppSettings::windowGeometry() const {
    return m_settings->value("window/geometry").toByteArray();
}

void AppSettings::setWindowGeometry(const QByteArray& geometry) {
    m_settings->setValue("window/geometry", geometry);
}

QByteArray AppSettings::windowState() const {
    return m_settings->value("window/state").toByteArray();
}

void AppSettings::setWindowState(const QByteArray& state) {
    m_settings->setValue("window/state", state);
}
