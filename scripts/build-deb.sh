#!/usr/bin/env bash
set -euo pipefail

# MauiKit software MUST be built with CMake and ECM; NEVER use Meson.

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$repo_root"
PACKAGE_VERSION="${PACKAGE_VERSION:?PACKAGE_VERSION must be supplied by CI}"
TARGET_ARCH="${TARGET_ARCH:-$(dpkg --print-architecture)}"
DEB_HOST_MULTIARCH="${DEB_HOST_MULTIARCH:-$(dpkg-architecture -qDEB_HOST_MULTIARCH)}"

source_dir="$repo_root"

build_dir="$repo_root/build"
stage_dir="$build_dir/package"
rm -rf "$build_dir" "$stage_dir"
mkdir -p "$build_dir"

cmake -S "$source_dir" -B "$build_dir" \
    -DCMAKE_INSTALL_PREFIX=/usr \
    -DKDE_INSTALL_LIBDIR="lib/$DEB_HOST_MULTIARCH" \
    -DCMAKE_BUILD_TYPE=Release
cmake --build "$build_dir" --parallel "$(nproc)"
DESTDIR="$stage_dir" cmake --install "$build_dir"

mkdir -p "$stage_dir/DEBIAN"
cat > "$stage_dir/DEBIAN/control" <<CONTROL
Package: maui-settings
Version: ${PACKAGE_VERSION}
Section: utils
Priority: optional
Architecture: ${TARGET_ARCH}
Maintainer: uri_herrera@nxos.org
Provides: maui-settings
Depends: libkf6bluezqt6,libkf6coreaddons6,libkf6i18n6,libkf6configgui6,libqt6core6t64,libqt6dbus6,libqt6gui6,libqt6network6,libqt6qml6,libqt6quick6,libqt6widgets6,cracklib-runtime,hypridle,hyprland,hyprpaper,libcrack2,libgtk-3-0,libgtk-4-1,libkf6authcore6,libkf6colorscheme6,libkf6configcore6,libkf6modemmanagerqt6,libkf6networkmanagerqt6,libnotify-bin,libpwquality-common,libpwquality1,libqcoro6core0,libqcoro6dbus0,libqcoro6network0,libqcoro6qml0,libqcoro6quick0,libqcoro6websockets0,libqt6svg6,libqt6keychain1,libxcursor1,mauikit (>= 4.0.5),mauikit-calendar (>= 4.0.5),mauikit-filebrowsing (>= 4.0.5),mauikit-system (>= 4.0.5),qml6-module-qtquick,qml6-module-qtquick-controls,qml6-module-qtquick-effects,qml6-module-qtquick-layouts,nwsm,nx-powerd,openrc,python3-envycontrol,wamerican
Description: maui-settings
 maui-settings
CONTROL

dpkg-deb --build --root-owner-group "$stage_dir" "$build_dir/maui-settings_${PACKAGE_VERSION}_${TARGET_ARCH}.deb"
