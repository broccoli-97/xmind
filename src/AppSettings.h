#pragma once

#include <QByteArray>
#include <QObject>

class QSettings;

enum class AppTheme { Light, Dark };

class AppSettings : public QObject {
    Q_OBJECT

public:
    static AppSettings& instance();

    AppTheme theme() const;
    void setTheme(AppTheme theme);

    bool autoSaveEnabled() const;
    void setAutoSaveEnabled(bool enabled);

    int autoSaveIntervalMinutes() const;
    void setAutoSaveIntervalMinutes(int minutes);

    int defaultFontSize() const;
    void setDefaultFontSize(int size);

    QByteArray windowGeometry() const;
    void setWindowGeometry(const QByteArray& geometry);

    QByteArray windowState() const;
    void setWindowState(const QByteArray& state);

signals:
    void themeChanged(AppTheme theme);
    void autoSaveSettingsChanged();
    void defaultFontSizeChanged(int size);

private:
    AppSettings();
    ~AppSettings() override;
    AppSettings(const AppSettings&) = delete;
    AppSettings& operator=(const AppSettings&) = delete;

    QSettings* m_settings;
};
