#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$repo_root"
PACKAGE_VERSION="${PACKAGE_VERSION:-0.0.1}"
TARGET_ARCH="${TARGET_ARCH:-$(dpkg --print-architecture)}"

source_dir="$repo_root"

build_dir="$repo_root/build"
meson_dir="$build_dir/meson"
stage_dir="$build_dir/package"
rm -rf "$meson_dir" "$stage_dir"
mkdir -p "$build_dir"

meson_args=(--prefix=/usr --buildtype=release)
if [ "$(dpkg-architecture -qDEB_HOST_ARCH)" = "amd64" ]; then
    meson_args+=("-Dcpp_args=-march=x86-64-v3")
fi

meson setup "$meson_dir" "$source_dir" "${meson_args[@]}"
meson compile -C "$meson_dir" -j "$(nproc)"
DESTDIR="$stage_dir" meson install -C "$meson_dir"

mkdir -p "$stage_dir/DEBIAN"
cat > "$stage_dir/DEBIAN/control" <<CONTROL
Package: maui-settings
Version: ${PACKAGE_VERSION}
Section: utils
Priority: optional
Architecture: ${TARGET_ARCH}
Maintainer: uri_herrera@nxos.org
Provides: maui-settings
Depends: cracklib-runtime,hypridle,hyprland,hyprpaper,libcrack2,libgtk-3-0,libgtk-4-1,libkf6authcore6,libkf6colorscheme6,libkf6configcore6,libkf6modemmanagerqt6,libkf6networkmanagerqt6,libnotify-bin,libpwquality-common,libpwquality1,libqcoro6core0,libqcoro6dbus0,libqcoro6network0,libqcoro6qml0,libqcoro6quick0,libqcoro6websockets0,libqt6svg6,libqt6keychain1,libxcursor1,mauikit (>= 4.0.5),mauikit-filebrowsing (>= 4.0.5),mauikit-system (>= 4.0.5),nwsm,nx-powerd,openrc,python3-envycontrol,wamerican
Description: maui-settings
 maui-settings
CONTROL

dpkg-deb --build --root-owner-group "$stage_dir" "$build_dir/maui-settings_${PACKAGE_VERSION}_${TARGET_ARCH}.deb"
