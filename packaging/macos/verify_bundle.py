#!/usr/bin/env python3
"""Reject incomplete or host-dependent macOS application bundles."""

import argparse
import configparser
import os
from pathlib import Path
import plistlib
import re
import struct
import subprocess


def run(*args):
    return subprocess.check_output(args, text=True, stderr=subprocess.STDOUT)


def version(value):
    parts = tuple(int(part) for part in value.split("."))
    return parts + (0,) * (3 - len(parts))


def require(condition, message):
    if not condition:
        raise RuntimeError(message)


def is_macho(path):
    with path.open("rb") as stream:
        return stream.read(4) in (
            b"\xfe\xed\xfa\xce", b"\xce\xfa\xed\xfe",
            b"\xfe\xed\xfa\xcf", b"\xcf\xfa\xed\xfe",
            b"\xca\xfe\xba\xbe", b"\xbe\xba\xfe\xca",
            b"\xca\xfe\xba\xbf", b"\xbf\xba\xfe\xca",
        )


def check_icon(path):
    data = path.read_bytes()
    require(data[:4] == b"icns" and struct.unpack(">I", data[4:8])[0] == len(data),
            "Invalid ICNS header")
    offset, kinds = 8, set()
    while offset < len(data):
        kind, size = struct.unpack(">4sI", data[offset:offset + 8])
        require(size > 8 and offset + size <= len(data), "Invalid ICNS image block")
        kinds.add(kind)
        offset += size
    require({b"ic07", b"ic08", b"ic09", b"ic10"} <= kinds,
            "Icon is missing standard or Retina resolutions")


def rpaths(commands):
    return re.findall(r"cmd LC_RPATH\n.*?\n\s*path (.*?) \(offset", commands)


def expand(path, binary, executable):
    return Path(path.replace("@loader_path", str(binary.parent)).replace(
        "@executable_path", str(executable.parent))).resolve()


def verify(app, expected_minimum=None, architectures=None):
    app = app.resolve()
    contents = app / "Contents"
    plist = plistlib.loads((contents / "Info.plist").read_bytes())
    for key in ("CFBundleExecutable", "CFBundleIdentifier", "CFBundleName",
                "CFBundleIconFile", "CFBundleVersion", "CFBundleShortVersionString",
                "LSMinimumSystemVersion"):
        require(plist.get(key), f"Missing Info.plist key: {key}")
    require(plist.get("CFBundlePackageType") == "APPL", "Not an application bundle")
    for key in ("CFBundleVersion", "CFBundleShortVersionString"):
        require(re.fullmatch(r"\d+\.\d+\.\d+", plist[key]), f"Invalid {key}")
    minimum = plist["LSMinimumSystemVersion"]
    if expected_minimum:
        require(version(minimum) == version(expected_minimum),
                f"Expected macOS {expected_minimum}, found {minimum}")
    check_icon(contents / "Resources" / plist["CFBundleIconFile"])

    gui = contents / "MacOS" / plist["CFBundleExecutable"]
    cli = contents / "MacOS" / "ymind-cli"
    for binary in (gui, cli):
        require(binary.is_file() and os.access(binary, os.X_OK),
                f"Missing executable: {binary}")
    for plugin in ("platforms/libqcocoa.dylib", "platforms/libqoffscreen.dylib",
                   "imageformats/libqsvg.dylib",
                   "tls/libqsecuretransportbackend.dylib"):
        require((contents / "PlugIns" / plugin).is_file(), f"Missing Qt plugin: {plugin}")
    # Qt 6.7 compiles Cocoa printer support directly into QtPrintSupport;
    # unlike the Linux CUPS backend, it is not a separate plugin.
    require((contents / "Frameworks/QtPrintSupport.framework/QtPrintSupport").is_file(),
            "Missing QtPrintSupport framework (Cocoa printing and PDF export)")
    for catalog in ("qt_zh_CN.qm", "qtbase_zh_CN.qm"):
        require((contents / "Resources/translations" / catalog).is_file(),
                f"Missing Chinese Qt dialog translations: {catalog}")
    config = configparser.ConfigParser()
    config.read(contents / "Resources/qt.conf")
    require(config.get("Paths", "Plugins", fallback="") == "PlugIns",
            "qt.conf does not select bundled plugins")

    documents = plist.get("CFBundleDocumentTypes", [])
    require(any(d.get("CFBundleTypeRole") == "Editor" for d in documents),
            "Missing Finder document association")

    architectures = architectures or run("lipo", "-archs", str(gui)).split()
    binaries = {p.resolve() for p in contents.rglob("*") if p.is_file() and is_macho(p)}
    for arch in architectures:
        executable_paths = {}
        for executable in (gui, cli):
            commands = run("otool", "-arch", arch, "-l", str(executable))
            executable_paths[executable.resolve()] = [
                expand(p, executable, executable) for p in rpaths(commands)]
        for binary in sorted(binaries):
            require(arch in run("lipo", "-archs", str(binary)).split(),
                    f"Missing {arch} architecture: {binary}")
            commands = run("otool", "-arch", arch, "-l", str(binary))
            targets = re.findall(r"^\s*(?:minos|version) (\d+(?:\.\d+)+)\s*$", commands, re.M)
            require(targets, f"Missing macOS deployment target: {binary}")
            require(all(version(v) <= version(minimum) for v in targets),
                    f"{binary.name} ({arch}) requires macOS {max(targets, key=version)}, "
                    f"but Info.plist advertises {minimum}")
            local_paths = [expand(p, binary, gui) for p in rpaths(commands)]
            dependencies = run("otool", "-arch", arch, "-L", str(binary))
            for dep in re.findall(r"^\s+(.+?) \(compatibility version", dependencies, re.M):
                if dep.startswith(("/System/Library/", "/usr/lib/")):
                    continue
                if dep.startswith("@rpath/"):
                    root = cli if binary == cli.resolve() else gui
                    candidates = [base / dep[len("@rpath/"):] for base in
                                  local_paths + executable_paths[root.resolve()]]
                else:
                    candidates = [expand(dep, binary, gui)]
                require(any(p.is_file() and app in p.resolve().parents for p in candidates),
                        f"Unbundled dependency in {binary.name}: {dep}")

    run("plutil", "-lint", str(contents / "Info.plist"))
    run("codesign", "--verify", "--deep", "--strict", str(app))
    print(f"Verified {app.name}: macOS {minimum}+, {', '.join(architectures)}, "
          f"{len(binaries)} Mach-O files, icons, plugins, translations and signatures")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("app", type=Path)
    parser.add_argument("--minimum")
    parser.add_argument("--architectures", nargs="+")
    args = parser.parse_args()
    verify(args.app, args.minimum, args.architectures)
