#include "cli/McpServer.h"

#include "cli/Renderer.h"
#include "core/ThemeDescriptor.h"
#include "core/ThemeRegistry.h"
#include "scene/MindMapScene.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTextStream>

#include <cstdio>

namespace McpServer {

namespace {

constexpr const char* kProtocolVersion = "2024-11-05";
constexpr const char* kServerName = "ymind";

// JSON-RPC 2.0 error codes
constexpr int kParseError = -32700;
constexpr int kInvalidRequest = -32600;
constexpr int kMethodNotFound = -32601;
constexpr int kInvalidParams = -32602;
constexpr int kInternalError = -32603;

QJsonObject makeError(int code, const QString& message) {
    QJsonObject e;
    e["code"] = code;
    e["message"] = message;
    return e;
}

QJsonObject makeResponse(const QJsonValue& id, const QJsonObject& result) {
    QJsonObject r;
    r["jsonrpc"] = "2.0";
    r["id"] = id;
    r["result"] = result;
    return r;
}

QJsonObject makeErrorResponse(const QJsonValue& id, const QJsonObject& error) {
    QJsonObject r;
    r["jsonrpc"] = "2.0";
    r["id"] = id;
    r["error"] = error;
    return r;
}

// Build the static tool catalogue once per process — schemas don't change at
// runtime. JSON-Schema flavour follows the MCP examples.
QJsonArray buildToolList() {
    QJsonArray tools;

    // render_mindmap
    {
        QJsonObject schema;
        schema["type"] = "object";
        QJsonObject props;
        {
            QJsonObject p;
            p["type"] = "string";
            p["description"] =
                "Markdown source. ATX headings (#, ##, ...) and nested lists "
                "(-, *, +) define the hierarchy. The first heading becomes the "
                "root topic.";
            props["markdown"] = p;
        }
        {
            QJsonObject p;
            p["type"] = "string";
            p["enum"] = QJsonArray{"svg", "png"};
            p["default"] = "svg";
            p["description"] =
                "Output format. SVG is returned as text; PNG is returned as "
                "base64-encoded image content.";
            props["format"] = p;
        }
        {
            QJsonObject p;
            p["type"] = "string";
            p["enum"] = QJsonArray{"bilateral", "topdown", "righttree"};
            p["default"] = "bilateral";
            p["description"] = "Auto-layout algorithm.";
            props["layout"] = p;
        }
        {
            QJsonObject p;
            p["type"] = "string";
            p["description"] =
                "Theme id (call list_themes for available ids). Omit for the "
                "default theme.";
            props["theme"] = p;
        }
        {
            QJsonObject p;
            p["type"] = "integer";
            p["default"] = 2;
            p["minimum"] = 1;
            p["maximum"] = 4;
            p["description"] = "PNG scale factor (1-4). Ignored for SVG.";
            props["scale"] = p;
        }
        schema["properties"] = props;
        schema["required"] = QJsonArray{"markdown"};

        QJsonObject tool;
        tool["name"] = "render_mindmap";
        tool["description"] =
            "Render a Markdown document as an auto-laid-out mindmap image "
            "(SVG or PNG). Use ATX headings for hierarchy and list items for "
            "leaves.";
        tool["inputSchema"] = schema;
        tools.append(tool);
    }

    // list_layouts
    {
        QJsonObject schema;
        schema["type"] = "object";
        schema["properties"] = QJsonObject{};

        QJsonObject tool;
        tool["name"] = "list_layouts";
        tool["description"] = "Return the supported layout algorithm names.";
        tool["inputSchema"] = schema;
        tools.append(tool);
    }

    // list_themes
    {
        QJsonObject schema;
        schema["type"] = "object";
        schema["properties"] = QJsonObject{};

        QJsonObject tool;
        tool["name"] = "list_themes";
        tool["description"] =
            "Return the available themes — each entry has an id, a display "
            "name, and an optional description.";
        tool["inputSchema"] = schema;
        tools.append(tool);
    }

    return tools;
}

QJsonObject textContent(const QString& text) {
    QJsonObject c;
    c["type"] = "text";
    c["text"] = text;
    return c;
}

QJsonObject imageContent(const QByteArray& bytes, const QString& mime) {
    QJsonObject c;
    c["type"] = "image";
    c["data"] = QString::fromLatin1(bytes.toBase64());
    c["mimeType"] = mime;
    return c;
}

QJsonObject errorToolResult(const QString& message) {
    QJsonObject result;
    QJsonArray content;
    content.append(textContent(message));
    result["content"] = content;
    result["isError"] = true;
    return result;
}

QJsonObject callRenderMindmap(const QJsonObject& args) {
    const QString md = args.value("markdown").toString();
    if (md.trimmed().isEmpty())
        return errorToolResult(QStringLiteral("`markdown` argument is required"));

    const QString layout = args.value("layout").toString(QStringLiteral("bilateral"));
    const QString theme = args.value("theme").toString();
    const QString fmtStr = args.value("format").toString(QStringLiteral("svg"));
    int scale = args.value("scale").toInt(2);
    if (scale < 1) scale = 1;
    if (scale > 4) scale = 4;

    bool fmtOk = false;
    Renderer::Format fmt = Renderer::formatFromString(fmtStr, &fmtOk);
    if (!fmtOk)
        return errorToolResult(
            QStringLiteral("`format` must be 'svg' or 'png', got: %1").arg(fmtStr));

    MindMapScene scene;
    QByteArray bytes;
    QString err;
    if (!Renderer::renderMarkdown(&scene, md, layout, theme, fmt, scale, bytes,
                                  &err)) {
        return errorToolResult(QStringLiteral("render failed: %1").arg(err));
    }

    QJsonObject result;
    QJsonArray content;
    if (fmt == Renderer::Format::Svg)
        content.append(textContent(QString::fromUtf8(bytes)));
    else
        content.append(imageContent(bytes, Renderer::formatToMime(fmt)));
    result["content"] = content;
    return result;
}

QJsonObject callListLayouts(const QJsonObject& /*args*/) {
    QJsonArray names{"bilateral", "topdown", "righttree"};
    QJsonObject payload;
    payload["layouts"] = names;

    QJsonObject result;
    QJsonArray content;
    content.append(textContent(QString::fromUtf8(
        QJsonDocument(payload).toJson(QJsonDocument::Compact))));
    result["content"] = content;
    return result;
}

QJsonObject callListThemes(const QJsonObject& /*args*/) {
    QJsonArray themes;
    for (const auto* td : ThemeRegistry::instance().allThemes()) {
        QJsonObject t;
        t["id"] = td->id;
        t["name"] = td->name;
        if (!td->description.isEmpty())
            t["description"] = td->description;
        themes.append(t);
    }
    QJsonObject payload;
    payload["themes"] = themes;

    QJsonObject result;
    QJsonArray content;
    content.append(textContent(QString::fromUtf8(
        QJsonDocument(payload).toJson(QJsonDocument::Compact))));
    result["content"] = content;
    return result;
}

QJsonObject dispatchToolCall(const QJsonObject& params) {
    const QString name = params.value("name").toString();
    const QJsonObject args = params.value("arguments").toObject();

    if (name == QLatin1String("render_mindmap"))
        return callRenderMindmap(args);
    if (name == QLatin1String("list_layouts"))
        return callListLayouts(args);
    if (name == QLatin1String("list_themes"))
        return callListThemes(args);
    return errorToolResult(QStringLiteral("unknown tool: %1").arg(name));
}

} // namespace

QJsonObject handleMessage(const QJsonObject& request) {
    const QString method = request.value("method").toString();
    const QJsonValue id = request.value("id");
    const QJsonObject params = request.value("params").toObject();

    // Notifications (id absent) require no response, but the caller filters
    // those out. Just return an empty object as a sentinel — `run()` checks
    // for an empty response and skips emitting.
    if (id.isUndefined() || id.isNull())
        return QJsonObject{};

    if (method == QLatin1String("initialize")) {
        QJsonObject caps;
        caps["tools"] = QJsonObject{};

        QJsonObject info;
        info["name"] = kServerName;
        info["version"] = QStringLiteral(YMIND_VERSION);

        QJsonObject result;
        result["protocolVersion"] = kProtocolVersion;
        result["capabilities"] = caps;
        result["serverInfo"] = info;
        result["instructions"] =
            "Use render_mindmap to turn Markdown into an auto-laid-out "
            "mindmap (SVG/PNG). Call list_layouts and list_themes to "
            "discover supported options.";
        return makeResponse(id, result);
    }

    if (method == QLatin1String("ping"))
        return makeResponse(id, QJsonObject{});

    if (method == QLatin1String("tools/list")) {
        QJsonObject result;
        result["tools"] = buildToolList();
        return makeResponse(id, result);
    }

    if (method == QLatin1String("tools/call")) {
        QJsonObject toolResult = dispatchToolCall(params);
        return makeResponse(id, toolResult);
    }

    return makeErrorResponse(id, makeError(kMethodNotFound,
                                           QStringLiteral("method not found: %1")
                                               .arg(method)));
}

int run() {
    // Use plain stdio: line-buffered, UTF-8 in/out. QTextStream over stdin
    // works fine here because MCP framing is one JSON object per line.
    QTextStream in(stdin);
    in.setEncoding(QStringConverter::Utf8);

    while (!in.atEnd()) {
        const QString line = in.readLine();
        if (line.isEmpty())
            continue;

        QJsonParseError perr;
        QJsonDocument doc = QJsonDocument::fromJson(line.toUtf8(), &perr);
        if (perr.error != QJsonParseError::NoError || !doc.isObject()) {
            QJsonObject err = makeErrorResponse(
                QJsonValue::Null,
                makeError(kParseError,
                          QStringLiteral("parse error: %1").arg(perr.errorString())));
            std::fwrite(QJsonDocument(err).toJson(QJsonDocument::Compact).constData(),
                        1, static_cast<size_t>(QJsonDocument(err).toJson(QJsonDocument::Compact).size()),
                        stdout);
            std::fputc('\n', stdout);
            std::fflush(stdout);
            continue;
        }

        QJsonObject response;
        try {
            response = handleMessage(doc.object());
        } catch (...) {
            response = makeErrorResponse(
                doc.object().value("id"),
                makeError(kInternalError, QStringLiteral("uncaught exception")));
        }

        if (response.isEmpty())
            continue; // notification — no response required

        const QByteArray payload = QJsonDocument(response).toJson(QJsonDocument::Compact);
        std::fwrite(payload.constData(), 1, static_cast<size_t>(payload.size()), stdout);
        std::fputc('\n', stdout);
        std::fflush(stdout);
    }

    return 0;
}

} // namespace McpServer
