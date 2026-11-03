# macOS packaging

The CMake GUI target is a real `YMind.app` bundle. Its Info.plist, executable
name, version and minimum macOS version are generated together. The icon uses
the existing YMind application logo, with sizes from 16 through 1024 pixels.
`resources/icons/ymind.svg` is the vector source; `ymind.png` and `YMind.icns`
are committed so builds need no image conversion tools.

Official releases use Qt 6.7.3 and macOS 11.0 as the deployment target on both
arm64 and x86_64. Qt documents this baseline in its
[supported platforms](https://doc.qt.io/archives/qt-6.7/supported-platforms.html#macos).
The deployment target is set before CMake initializes its compiler. Override
it explicitly when using a Qt kit requiring a newer system.

From the repository root, after building on macOS:

```bash
bash packaging/macos/package.sh build dist
python3 packaging/macos/verify_bundle.py dist/YMind.app --minimum 11.0
python3 packaging/macos/smoke_test.py dist/YMind.app
```

Use a fresh staging directory. `package.sh` installs the GUI and companion CLI,
deploys their frameworks and plugins (including offscreen for CLI/MCP), copies
Qt's Chinese translation catalogs and writes `qt.conf`. It signs only after
all binary and resource changes are finished.

`verify_bundle.py` checks bundle metadata, Finder association, executable
permissions, ICNS resolutions, Cocoa/offscreen/SVG/TLS plugins, QtPrintSupport,
translations, architectures, Mach-O minimum system versions, dependency
resolution and signature validity. `smoke_test.py` starts the native Cocoa GUI
and exercises CLI rendering and MCP without Qt discovery environment variables.
CI runs both scripts against a relocated bundle on Apple Silicon and Intel.
The C++ tests cover Finder file-open events, startup event buffering, Unicode
paths and switching to an already open document.

## Signing and notarization

Default builds have an ad-hoc signature, which makes the modified Apple Silicon
binaries valid but does not establish a trusted publisher. Downloaded ad-hoc
packages may require an explicit first-launch allowance in System Settings →
Privacy & Security. CI does not claim notarization without a signing identity.

For trusted distribution, install a Developer ID Application certificate in the
build machine's Keychain, store a `notarytool` credentials profile, then run:

```bash
export MACOS_CODESIGN_IDENTITY='Developer ID Application: Your Name (TEAMID)'
export MACOS_NOTARY_PROFILE='ymind-notary'
bash packaging/macos/package.sh build dist
```

The script submits the signed bundle, waits for notarization, and staples and
validates the ticket before returning. Keep certificate and Apple credentials
outside the repository. Create distribution archives only after this completes.

## Manual compatibility check

CI runners use a newer macOS host. Mach-O checks prevent accidentally raising
the deployment target, but running on macOS 11 still requires a macOS 11 machine.
On that machine, install the matching architecture and check Finder launch,
opening `.ymind`, editing and saving, SVG/PNG/PDF export, Chinese input, and
`/Applications/YMind.app/Contents/MacOS/ymind-cli mcp`.
