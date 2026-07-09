#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="$PROJECT_DIR/build/trucCompBus_artefacts/Release"
OUT_DIR="$SCRIPT_DIR/output"
VERSION="1.1.0"

echo "=== trucComp@Bus $VERSION インストーラービルド ==="

mkdir -p "$OUT_DIR"

# ── VST3 コンポーネントパッケージ ──────────────────────────────────────────
VST3_ROOT="$OUT_DIR/pkg_vst3"
rm -rf "$VST3_ROOT"
mkdir -p "$VST3_ROOT/Library/Audio/Plug-Ins/VST3"
cp -r "$BUILD_DIR/VST3/trucComp@Bus.vst3" \
      "$VST3_ROOT/Library/Audio/Plug-Ins/VST3/"

pkgbuild \
    --root "$VST3_ROOT" \
    --identifier "com.truc.trucCompBus.vst3" \
    --version "$VERSION" \
    --install-location "/" \
    "$OUT_DIR/trucCompBus_VST3.pkg"

echo "✓ VST3パッケージ作成完了"

# ── AU コンポーネントパッケージ ────────────────────────────────────────────
AU_ROOT="$OUT_DIR/pkg_au"
rm -rf "$AU_ROOT"
mkdir -p "$AU_ROOT/Library/Audio/Plug-Ins/Components"
cp -r "$BUILD_DIR/AU/trucComp@Bus.component" \
      "$AU_ROOT/Library/Audio/Plug-Ins/Components/"

pkgbuild \
    --root "$AU_ROOT" \
    --identifier "com.truc.trucCompBus.au" \
    --version "$VERSION" \
    --install-location "/" \
    "$OUT_DIR/trucCompBus_AU.pkg"

echo "✓ AUパッケージ作成完了"

# ── 統合インストーラー ────────────────────────────────────────────────────
productbuild \
    --distribution "$SCRIPT_DIR/distribution.xml" \
    --resources "$SCRIPT_DIR/Resources" \
    --package-path "$OUT_DIR" \
    "$OUT_DIR/trucCompBus_${VERSION}_Installer.pkg"

echo ""
echo "=== 完了 ==="
echo "インストーラー: $OUT_DIR/trucCompBus_${VERSION}_Installer.pkg"

# 一時ファイル削除
rm -rf "$OUT_DIR/pkg_vst3" "$OUT_DIR/pkg_au" \
       "$OUT_DIR/trucCompBus_VST3.pkg" "$OUT_DIR/trucCompBus_AU.pkg"
