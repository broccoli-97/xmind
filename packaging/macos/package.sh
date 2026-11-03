#!/usr/bin/env bash
set -euo pipefail

# Run on macOS after building. Stage into a fresh directory so stale plugins or
# signatures cannot hide a deployment error. The build tree stays untouched.
build_dir=${1:-build}
stage_dir=${2:-dist}
if [[ $(uname -s) != Darwin ]]; then
    echo 'macOS packaging requires macOS and the Xcode command line tools.' >&2
    exit 1
fi
if [[ -e "$stage_dir/YMind.app" ]]; then
    echo "Use a fresh staging directory: $stage_dir/YMind.app already exists." >&2
    exit 1
fi

cmake --install "$build_dir" --config Release --prefix "$stage_dir"
app="$stage_dir/YMind.app"
plugins_dir=$(qtpaths --query QT_INSTALL_PLUGINS)
offscreen="$app/Contents/PlugIns/platforms/libqoffscreen.dylib"
mkdir -p "$(dirname "$offscreen")"
cp "$plugins_dir/platforms/libqoffscreen.dylib" "$offscreen"

# macdeployqt excludes offscreen by default. Passing it as an extra executable
# also deploys its dependencies and rewrites its library paths, just like CLI.
macdeployqt "$app" -always-overwrite -verbose=2 \
    "-executable=$app/Contents/MacOS/ymind-cli" \
    "-executable=$offscreen"

# macdeployqt 6.7 does not deploy .qm files. Include the Qt translation catalogs
# and their dependencies so native/standard dialogs also display Chinese.
translations_dir=$(qtpaths --query QT_INSTALL_TRANSLATIONS)
mkdir -p "$app/Contents/Resources/translations"
cp "$translations_dir"/*_zh_CN.qm "$app/Contents/Resources/translations/"

# Both executables live in Contents/MacOS and must use bundle-local plugins and
# translations even on machines that happen to have another Qt installation.
cat > "$app/Contents/Resources/qt.conf" <<'EOF'
[Paths]
Plugins = PlugIns
Translations = Resources/translations
EOF

# install_name_tool changes invalidate the original Apple Silicon signatures.
# Sign only after the entire bundle is deployed. A Developer ID can be supplied
# locally; the default ad-hoc signature does not imply Apple notarization.
identity=${MACOS_CODESIGN_IDENTITY:--}
sign_args=(--force --deep --sign "$identity")
if [[ "$identity" != - ]]; then
    sign_args+=(--options runtime --timestamp)
elif [[ -n ${MACOS_NOTARY_PROFILE:-} ]]; then
    echo 'Notarization requires a Developer ID Application signing identity.' >&2
    exit 1
fi
codesign "${sign_args[@]}" "$app"
codesign --verify --deep --strict --verbose=2 "$app"

# A configured Keychain profile enables real notarization for distribution.
# No credentials are stored in the repository or required for local builds.
if [[ -n ${MACOS_NOTARY_PROFILE:-} ]]; then
    notary_dir=$(mktemp -d -t ymind-notarize)
    archive="$notary_dir/YMind.zip"
    trap 'rm -f "$archive"; rmdir "$notary_dir"' EXIT
    ditto -c -k --keepParent "$app" "$archive"
    xcrun notarytool submit "$archive" --keychain-profile "$MACOS_NOTARY_PROFILE" --wait
    xcrun stapler staple "$app"
    xcrun stapler validate "$app"
fi

python3 "$(dirname "$0")/verify_bundle.py" "$app"
