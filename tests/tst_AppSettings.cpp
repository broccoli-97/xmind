#include "core/AppSettings.h"

#include <QSignalSpy>
#include <QTest>

class tst_AppSettings : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();

    void themeRoundTrip();
    void themeSignal();
    void autoSaveRoundTrip();
    void autoSaveSignal();
    void autoSaveIntervalClamped();
    void fontSizeRoundTrip();
    void fontSizeClamped();
    void fontSizeSignal();
    void fontFamilyRoundTrip();
    void fontFamilySignal();
};

void tst_AppSettings::initTestCase() {
    qRegisterMetaType<AppTheme>("AppTheme");
}

void tst_AppSettings::themeRoundTrip() {
    auto& s = AppSettings::instance();
    s.setTheme(AppTheme::Dark);
    QCOMPARE(s.theme(), AppTheme::Dark);
    s.setTheme(AppTheme::Light);
    QCOMPARE(s.theme(), AppTheme::Light);
}

void tst_AppSettings::themeSignal() {
    auto& s = AppSettings::instance();
    s.setTheme(AppTheme::Light); // ensure known state

    QSignalSpy spy(&s, &AppSettings::themeChanged);
    s.setTheme(AppTheme::Dark);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.first().first().value<AppTheme>(), AppTheme::Dark);

    // Setting same value again should NOT emit
    spy.clear();
    s.setTheme(AppTheme::Dark);
    QCOMPARE(spy.count(), 0);
}

void tst_AppSettings::autoSaveRoundTrip() {
    auto& s = AppSettings::instance();
    s.setAutoSaveEnabled(true);
    QCOMPARE(s.autoSaveEnabled(), true);
    s.setAutoSaveEnabled(false);
    QCOMPARE(s.autoSaveEnabled(), false);
}

void tst_AppSettings::autoSaveSignal() {
    auto& s = AppSettings::instance();
    QSignalSpy spy(&s, &AppSettings::autoSaveSettingsChanged);
    s.setAutoSaveEnabled(true);
    QVERIFY(spy.count() >= 1);
}

void tst_AppSettings::autoSaveIntervalClamped() {
    auto& s = AppSettings::instance();
    s.setAutoSaveIntervalMinutes(0);
    QCOMPARE(s.autoSaveIntervalMinutes(), 1);

    s.setAutoSaveIntervalMinutes(10);
    QCOMPARE(s.autoSaveIntervalMinutes(), 5);

    s.setAutoSaveIntervalMinutes(3);
    QCOMPARE(s.autoSaveIntervalMinutes(), 3);
}

void tst_AppSettings::fontSizeRoundTrip() {
    auto& s = AppSettings::instance();
    s.setDefaultFontSize(14);
    QCOMPARE(s.defaultFontSize(), 14);
}

void tst_AppSettings::fontSizeClamped() {
    auto& s = AppSettings::instance();
    s.setDefaultFontSize(2);
    QCOMPARE(s.defaultFontSize(), 8);

    s.setDefaultFontSize(100);
    QCOMPARE(s.defaultFontSize(), 24);
}

void tst_AppSettings::fontSizeSignal() {
    auto& s = AppSettings::instance();
    s.setDefaultFontSize(10); // ensure known state

    QSignalSpy spy(&s, &AppSettings::defaultFontSizeChanged);
    s.setDefaultFontSize(16);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.first().first().toInt(), 16);

    // Same value should NOT emit
    spy.clear();
    s.setDefaultFontSize(16);
    QCOMPARE(spy.count(), 0);
}

void tst_AppSettings::fontFamilyRoundTrip() {
    auto& s = AppSettings::instance();
    s.setDefaultFontFamily("Monospace");
    QCOMPARE(s.defaultFontFamily(), QString("Monospace"));
}

void tst_AppSettings::fontFamilySignal() {
    auto& s = AppSettings::instance();
    s.setDefaultFontFamily("Arial"); // known state

    QSignalSpy spy(&s, &AppSettings::defaultFontFamilyChanged);
    s.setDefaultFontFamily("Courier");
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.first().first().toString(), QString("Courier"));

    // Same value should NOT emit
    spy.clear();
    s.setDefaultFontFamily("Courier");
    QCOMPARE(spy.count(), 0);
}

QTEST_MAIN(tst_AppSettings)
#include "tst_AppSettings.moc"
