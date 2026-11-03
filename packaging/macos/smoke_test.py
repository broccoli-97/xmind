#!/usr/bin/env python3
"""Exercise the deployed CLI/MCP and Cocoa GUI without an installed Qt."""

import json
import os
from pathlib import Path
import plistlib
import subprocess
import sys
import tempfile
import xml.etree.ElementTree as ET


def smoke_test(app):
    contents = app.resolve() / "Contents"
    plist = plistlib.loads((contents / "Info.plist").read_bytes())
    cli = contents / "MacOS/ymind-cli"
    gui = contents / "MacOS" / plist["CFBundleExecutable"]
    # install-qt-action sets discovery paths. They must not rescue a broken app.
    env = {key: value for key, value in os.environ.items()
           if not key.startswith(("QT_", "QML", "DYLD_"))}

    def run_cli(*args, input=None):
        return subprocess.run([str(cli), *args], input=input, text=True, env=env,
                              check=True, capture_output=True, timeout=30).stdout

    assert "bilateral" in run_cli("list-layouts")
    assert "builtin.nord" in run_cli("list-themes")
    with tempfile.TemporaryDirectory(prefix="ymind macOS smoke ") as temp:
        temp = Path(temp)
        source = temp / "中文 mind map.md"
        source.write_text("# macOS smoke test\n## 中文节点\n### Child\n", encoding="utf-8")
        for fmt in ("svg", "png"):
            output = temp / f"map.{fmt}"
            run_cli("render", "-i", str(source), "-o", str(output), "-f", fmt)
            data = output.read_bytes()
            if fmt == "png":
                assert data.startswith(b"\x89PNG\r\n\x1a\n")
            else:
                assert ET.fromstring(data).tag.endswith("svg")

        request = json.dumps({"jsonrpc": "2.0", "id": 1, "method": "initialize",
                              "params": {"protocolVersion": "2024-11-05", "capabilities": {},
                                         "clientInfo": {"name": "smoke-test", "version": "1"}}})
        response = run_cli("mcp", input=request + "\n")
        assert any(json.loads(line).get("id") == 1 and "result" in json.loads(line)
                   for line in response.splitlines())

        # Use Cocoa (the GUI's default), rather than offscreen, to catch missing
        # native plugins and GUI-only startup crashes. Give delayed startup work
        # time to run, and capture diagnostics on failure.
        env["QT_QPA_PLATFORM"] = "cocoa"
        env["XDG_CONFIG_HOME"] = str(temp / "config")
        with (temp / "gui.log").open("w+") as log:
            proc = subprocess.Popen([str(gui)], cwd=temp, env=env, stdout=log, stderr=log)
            try:
                try:
                    proc.wait(timeout=6)
                except subprocess.TimeoutExpired:
                    pass
                else:
                    log.seek(0)
                    raise RuntimeError(f"Cocoa GUI exited during startup ({proc.returncode}):\n{log.read()}")
            finally:
                if proc.poll() is None:
                    proc.terminate()
                    proc.wait(timeout=10)
    print("Packaged CLI, SVG/PNG rendering, MCP initialization and Cocoa GUI startup passed")


if __name__ == "__main__":
    smoke_test(Path(sys.argv[1]))
