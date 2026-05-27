// Guards i18n integrity: every translatable UI string must have BOTH an
// English source and a non-empty Chinese translation. Without this guard a
// contributor can add a new tr("Some Button") in C++, forget to run lupdate
// or fill in the Chinese translation, and silently break the zh_CN locale
// for users who switch language at runtime.
//
// Two failure modes are covered here (static .ts parse). A third mode --
// brand-new tr() strings that were never added to the .ts at all -- is
// covered by a `lupdate` drift step in .github/workflows/ci.yml.

#include <QByteArray>
#include <QFile>
#include <QString>
#include <QStringList>
#include <QTest>
#include <QXmlStreamReader>

#ifndef YMIND_SOURCE_DIR
#error "YMIND_SOURCE_DIR must be defined so the test can locate the .ts file"
#endif

namespace {

QString tsFilePath() {
    return QStringLiteral(YMIND_SOURCE_DIR) + "/translations/ymind_zh_CN.ts";
}

struct MessageIssue {
    QString context;
    QString source;
    QString reason;

    QString format() const {
        QString src = source;
        if (src.length() > 80)
            src = src.left(77) + "...";
        return QStringLiteral("  [%1] %2 -- %3").arg(context, src, reason);
    }
};

}  // namespace

class tst_TranslationCoverage : public QObject {
    Q_OBJECT

private slots:
    void translationFileExists();
    void everyMessageHasEnglishAndChinese();
    void noUnfinishedOrVanishedEntries();
};

void tst_TranslationCoverage::translationFileExists() {
    const QString path = tsFilePath();
    QVERIFY2(QFile::exists(path),
             qPrintable(QStringLiteral("Translation file missing at: %1").arg(path)));
}

// Walks the .ts XML once and aggregates *every* defect before reporting,
// so a contributor sees the full list in one CI run instead of fixing them
// one at a time.
void tst_TranslationCoverage::everyMessageHasEnglishAndChinese() {
    QFile file(tsFilePath());
    QVERIFY2(file.open(QIODevice::ReadOnly | QIODevice::Text),
             qPrintable(QStringLiteral("Cannot open %1: %2").arg(file.fileName(), file.errorString())));

    QXmlStreamReader xml(&file);
    QList<MessageIssue> issues;
    int messageCount = 0;

    QString currentContext;
    QString currentSource;
    QString currentTranslation;
    bool inMessage = false;
    bool inContextName = false;
    bool inSource = false;
    bool inTranslation = false;

    while (!xml.atEnd()) {
        xml.readNext();
        if (xml.isStartElement()) {
            const QStringView name = xml.name();
            if (name == QStringLiteral("context")) {
                currentContext.clear();
            } else if (name == QStringLiteral("name")) {
                inContextName = true;
            } else if (name == QStringLiteral("message")) {
                inMessage = true;
                currentSource.clear();
                currentTranslation.clear();
            } else if (inMessage && name == QStringLiteral("source")) {
                inSource = true;
            } else if (inMessage && name == QStringLiteral("translation")) {
                inTranslation = true;
            }
        } else if (xml.isCharacters()) {
            if (inContextName)
                currentContext += xml.text().toString();
            else if (inSource)
                currentSource += xml.text().toString();
            else if (inTranslation)
                currentTranslation += xml.text().toString();
        } else if (xml.isEndElement()) {
            const QStringView name = xml.name();
            if (name == QStringLiteral("name")) {
                inContextName = false;
            } else if (name == QStringLiteral("source")) {
                inSource = false;
            } else if (name == QStringLiteral("translation")) {
                inTranslation = false;
            } else if (name == QStringLiteral("message")) {
                ++messageCount;
                if (currentSource.trimmed().isEmpty()) {
                    issues.append({currentContext, currentSource,
                                   QStringLiteral("missing English source")});
                }
                if (currentTranslation.trimmed().isEmpty()) {
                    issues.append({currentContext, currentSource,
                                   QStringLiteral("missing Chinese translation")});
                }
                inMessage = false;
            }
        }
    }

    QVERIFY2(!xml.hasError(),
             qPrintable(QStringLiteral("XML parse error at line %1: %2")
                            .arg(xml.lineNumber())
                            .arg(xml.errorString())));

    QVERIFY2(messageCount > 0,
             "Translation file contains no <message> entries -- something is wrong");

    if (!issues.isEmpty()) {
        QStringList lines;
        lines << QStringLiteral("Found %1 untranslated UI string(s):").arg(issues.size());
        for (const MessageIssue& issue : issues)
            lines << issue.format();
        lines << QStringLiteral("Run `lupdate` after adding tr() strings, then fill in "
                                "the Chinese translation in translations/ymind_zh_CN.ts.");
        QFAIL(qPrintable(lines.join('\n')));
    }
}

// `type="unfinished"` flags a Linguist-marked draft; `type="vanished"` flags
// a source string lupdate could no longer find. Either one in the committed
// .ts means a UI string is at risk of appearing in English when zh_CN is on.
void tst_TranslationCoverage::noUnfinishedOrVanishedEntries() {
    QFile file(tsFilePath());
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
    const QByteArray contents = file.readAll();

    QStringList offenders;
    if (contents.contains("type=\"unfinished\""))
        offenders << QStringLiteral("type=\"unfinished\" (draft translation)");
    if (contents.contains("type=\"vanished\""))
        offenders << QStringLiteral("type=\"vanished\" (stale source -- run `lupdate -no-obsolete`)");

    if (!offenders.isEmpty())
        QFAIL(qPrintable(QStringLiteral("translations/ymind_zh_CN.ts contains: %1")
                             .arg(offenders.join(", "))));
}

QTEST_APPLESS_MAIN(tst_TranslationCoverage)
#include "tst_TranslationCoverage.moc"
