#include "core/AiClient.h"
#include "core/AppSettings.h"

#include <QCoreApplication>
#include <QTest>

class tst_AiClient : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanMarkdownOutlineFences();
    void cleanMarkdownOutlinePlain();
    void appSettingsAiFields();
    void headerPlaceholdersExist();
};

void tst_AiClient::initTestCase() {
    QCoreApplication::setOrganizationName("YMindTest");
    QCoreApplication::setApplicationName("tst_AiClient");
}

void tst_AiClient::cleanMarkdownOutlineFences() {
    QString raw = "```markdown\n# Root Topic\n## Subtopic A\n- Item 1\n```";
    QString cleaned = AiClient::cleanMarkdownOutline(raw);
    QCOMPARE(cleaned, QString("# Root Topic\n## Subtopic A\n- Item 1"));

    QString raw2 = "```\n# Root Topic\n* Leaf\n```";
    QString cleaned2 = AiClient::cleanMarkdownOutline(raw2);
    QCOMPARE(cleaned2, QString("# Root Topic\n* Leaf"));
}

void tst_AiClient::cleanMarkdownOutlinePlain() {
    QString raw = "\n\n  # Main Idea\n## Sub 1\n## Sub 2  \n\n";
    QString cleaned = AiClient::cleanMarkdownOutline(raw);
    QCOMPARE(cleaned, QString("# Main Idea\n## Sub 1\n## Sub 2"));
}

void tst_AiClient::appSettingsAiFields() {
    auto& s = AppSettings::instance();
    s.setAiProvider("Custom");
    QCOMPARE(s.aiProvider(), QString("Custom"));

    s.setAiApiKey("sk-test-key-12345");
    QCOMPARE(s.aiApiKey(), QString("sk-test-key-12345"));

    s.setAiModel("qwen/qwen-2.5-72b-instruct:free");
    QCOMPARE(s.aiModel(), QString("qwen/qwen-2.5-72b-instruct:free"));

    s.setAiCustomEndpoint("https://my-custom-proxy.com/v1");
    QCOMPARE(s.aiCustomEndpoint(), QString("https://my-custom-proxy.com/v1"));
}

void tst_AiClient::headerPlaceholdersExist() {
    QVERIFY(strlen(AiClient::kDefaultOrcaEndpoint) > 0);
    QVERIFY(strlen(AiClient::kDefaultOrcaModel) > 0);
    QVERIFY(strlen(AiClient::kProjectReferer) > 0);
    QVERIFY(strlen(AiClient::kProjectTitle) > 0);
    QVERIFY(strlen(AiClient::kOrcaPartnerUrl) > 0);
}

QTEST_MAIN(tst_AiClient)
#include "tst_AiClient.moc"
