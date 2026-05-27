#include "cli/McpServer.h"
#include "cli/Renderer.h"
#include "core/RegistryStyleProvider.h"
#include "core/ThemeDescriptor.h"
#include "core/ThemeRegistry.h"
#include "core/TemplateRegistry.h"
#include "layout/LayoutAlgorithmRegistry.h"
#include "scene/MindMapScene.h"

#include <QApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QFile>
#include <QFontDatabase>
#include <QTextStream>

namespace {

QTextStream& cerr() {
    static QTextStream s(stderr);
    return s;
}

QTextStream& cout() {
    static QTextStream s(stdout);
    return s;
}

void printUsage() {
    cerr() << "ymind-cli " << YMIND_VERSION << " — headless mindmap rendering\n"
           << "\n"
           << "Usage: ymind-cli <command> [options]\n"
           << "\n"
           << "Commands:\n"
           << "  render        Convert Markdown to SVG/PNG\n"
           << "  list-layouts  Print supported layout algorithm names\n"
           << "  list-themes   Print available theme ids\n"
           << "  mcp           Run as a Model Context Protocol server over stdio\n"
           << "\n"
           << "Run `ymind-cli render --help` for render-specific options.\n";
    cerr().flush();
}

int runRender(QStringList args) {
    QCommandLineParser p;
    p.setApplicationDescription(
        "Render a Markdown document as a mindmap. Reads from --input (or stdin "
        "if '-') and writes to --output (or stdout if '-').");
    p.addHelpOption();

    QCommandLineOption inputOpt({"i", "input"},
                                "Input markdown file path. Use '-' for stdin.",
                                "path", "-");
    QCommandLineOption outputOpt({"o", "output"},
                                 "Output file path. Use '-' for stdout.",
                                 "path", "-");
    QCommandLineOption formatOpt({"f", "format"},
                                 "Output format: svg or png.", "format", "svg");
    QCommandLineOption layoutOpt("layout",
                                 "Layout: bilateral, topdown, righttree.",
                                 "name", "bilateral");
    QCommandLineOption themeOpt("theme", "Theme id (see `list-themes`).",
                                "id", "");
    QCommandLineOption scaleOpt("scale",
                                "PNG scale factor (default 2 = retina).",
                                "n", "2");
    p.addOptions({inputOpt, outputOpt, formatOpt, layoutOpt, themeOpt, scaleOpt});

    p.process(args);

    bool fmtOk = false;
    Renderer::Format fmt = Renderer::formatFromString(p.value(formatOpt), &fmtOk);
    if (!fmtOk) {
        cerr() << "error: unknown format '" << p.value(formatOpt)
               << "' (expected svg or png)\n";
        return 2;
    }

    int scale = p.value(scaleOpt).toInt();
    if (scale <= 0)
        scale = 2;

    // Read input
    QString markdown;
    const QString inPath = p.value(inputOpt);
    if (inPath == QLatin1String("-")) {
        QTextStream in(stdin);
        markdown = in.readAll();
    } else {
        QFile f(inPath);
        if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
            cerr() << "error: cannot read input file: " << inPath << "\n";
            return 2;
        }
        markdown = QString::fromUtf8(f.readAll());
    }

    if (markdown.trimmed().isEmpty()) {
        cerr() << "error: input is empty\n";
        return 2;
    }

    MindMapScene scene;
    QByteArray bytes;
    QString err;
    if (!Renderer::renderMarkdown(&scene, markdown, p.value(layoutOpt),
                                  p.value(themeOpt), fmt, scale, bytes, &err)) {
        cerr() << "error: " << err << "\n";
        return 3;
    }

    const QString outPath = p.value(outputOpt);
    if (outPath == QLatin1String("-")) {
        QFile out;
        if (!out.open(stdout, QIODevice::WriteOnly)) {
            cerr() << "error: cannot write to stdout\n";
            return 4;
        }
        out.write(bytes);
        out.close();
    } else {
        QFile out(outPath);
        if (!out.open(QIODevice::WriteOnly)) {
            cerr() << "error: cannot write output file: " << outPath << "\n";
            return 4;
        }
        out.write(bytes);
        out.close();
    }
    return 0;
}

int runListLayouts() {
    // The supported algorithm names are fixed (see LayoutStyle.h).
    cout() << "bilateral\n"
           << "topdown\n"
           << "righttree\n";
    cout().flush();
    return 0;
}

int runListThemes() {
    const auto themes = ThemeRegistry::instance().allThemes();
    for (const auto* td : themes) {
        cout() << td->id << "\t" << td->name << "\n";
    }
    cout().flush();
    return 0;
}

} // namespace

int main(int argc, char* argv[]) {
    // Force the offscreen platform plugin so a missing display doesn't kill
    // the binary in CI / headless environments. Caller can still override
    // via -platform on the command line if needed.
    if (qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM"))
        qputenv("QT_QPA_PLATFORM", "offscreen");

    QApplication app(argc, argv);
    app.setOrganizationName(QStringLiteral("YMind"));
    app.setApplicationName(QStringLiteral("ymind-cli"));
    app.setApplicationVersion(QStringLiteral(YMIND_VERSION));

    QFontDatabase::addApplicationFont(QStringLiteral(":/fonts/Caveat.ttf"));

    TemplateRegistry::instance().loadBuiltins();
    LayoutAlgorithmRegistry::instance().registerBuiltins();
    ThemeRegistry::instance().loadBuiltins();
    MindMapScene::setDefaultStyleProvider(&RegistryStyleProvider::instance());

    QStringList args = app.arguments();
    if (args.size() < 2) {
        printUsage();
        return 1;
    }

    const QString sub = args[1];
    // Peel the subcommand off so QCommandLineParser sees a clean list.
    args.removeAt(1);

    if (sub == QLatin1String("render"))
        return runRender(args);
    if (sub == QLatin1String("list-layouts"))
        return runListLayouts();
    if (sub == QLatin1String("list-themes"))
        return runListThemes();
    if (sub == QLatin1String("mcp"))
        return McpServer::run();
    if (sub == QLatin1String("--help") || sub == QLatin1String("-h")
        || sub == QLatin1String("help")) {
        printUsage();
        return 0;
    }
    if (sub == QLatin1String("--version") || sub == QLatin1String("-v")) {
        cout() << "ymind-cli " << YMIND_VERSION << "\n";
        cout().flush();
        return 0;
    }

    cerr() << "error: unknown command '" << sub << "'\n\n";
    printUsage();
    return 1;
}
