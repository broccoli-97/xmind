#include "core/ThemeRegistry.h"

#include <QTest>

class tst_ThemeRegistry : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();

    void builtinsAreLoaded();
    void defaultThemeIsPresent();
    void allThemesIncludesAllBuiltins();
    void registerCustomTheme();
};

void tst_ThemeRegistry::initTestCase() {
    ThemeRegistry::instance().loadBuiltins();
}

void tst_ThemeRegistry::builtinsAreLoaded() {
    QVERIFY(ThemeRegistry::instance().themeById("builtin.default") != nullptr);
    QVERIFY(ThemeRegistry::instance().themeById("builtin.morandi") != nullptr);
    QVERIFY(ThemeRegistry::instance().themeById("builtin.outlined") != nullptr);
    QVERIFY(ThemeRegistry::instance().themeById("builtin.tinted") != nullptr);
    // Lined is a template, not a theme.
    QVERIFY(ThemeRegistry::instance().themeById("builtin.lined") == nullptr);
}

void tst_ThemeRegistry::defaultThemeIsPresent() {
    const auto* th =
        ThemeRegistry::instance().themeById(ThemeRegistry::defaultThemeId());
    QVERIFY(th != nullptr);
}

void tst_ThemeRegistry::allThemesIncludesAllBuiltins() {
    auto all = ThemeRegistry::instance().allThemes();
    // 4 built-in themes: Default, Morandi, Outlined, Tinted.
    QVERIFY(all.size() >= 4);
}

void tst_ThemeRegistry::registerCustomTheme() {
    ThemeDescriptor td;
    td.id = "test.customtheme";
    td.name = "Custom";

    ThemeRegistry::instance().registerTheme(td);

    const auto* found = ThemeRegistry::instance().themeById("test.customtheme");
    QVERIFY(found != nullptr);
    QCOMPARE(found->name, QString("Custom"));
}

QTEST_APPLESS_MAIN(tst_ThemeRegistry)
#include "tst_ThemeRegistry.moc"
