#pragma once

#include <QJsonObject>
#include <QJsonValue>
#include <QString>

// Minimal Model Context Protocol (MCP) server speaking JSON-RPC 2.0 over
// stdio. Messages are newline-delimited, one JSON object per line — the
// canonical MCP stdio framing.
//
// Implemented methods:
//   - initialize           Handshake; advertises tool capabilities.
//   - notifications/initialized   No-op acknowledgement from the client.
//   - tools/list           Returns the catalogue of callable tools.
//   - tools/call           Dispatches to a named tool.
//   - ping                 Liveness probe.
//
// Exposed tools:
//   - render_mindmap       Convert a Markdown document into an SVG or PNG
//                          mindmap. Returns embedded image content.
//   - list_layouts         Returns supported layout algorithm names.
//   - list_themes          Returns available themes and whether they're dark.
namespace McpServer {

// Blocks reading stdin until EOF. Returns the process exit code (always 0
// under normal shutdown).
int run();

// Exposed for testing / single-message dispatch.
QJsonObject handleMessage(const QJsonObject& request);

} // namespace McpServer
