#! /bin/bash

set -x

### Update sources

wget -qO /etc/apt/sources.list.d/neon-user-repo.list https://raw.githubusercontent.com/Nitrux/iso-tool/development/configs/files/sources.list.neon.user

wget -qO /etc/apt/sources.list.d/nitrux-main-compat-repo.list https://raw.githubusercontent.com/Nitrux/iso-tool/development/configs/files/sources.list.nitrux

wget -qO /etc/apt/sources.list.d/nitrux-testing-repo.list https://raw.githubusercontent.com/Nitrux/iso-tool/development/configs/files/sources.list.nitrux.testing

DEBIAN_FRONTEND=noninteractive apt-key adv --keyserver keyserver.ubuntu.com --recv-keys \
	55751E5D > /dev/null

curl -L https://packagecloud.io/nitrux/repo/gpgkey | apt-key add -;
curl -L https://packagecloud.io/nitrux/compat/gpgkey | apt-key add -;
curl -L https://packagecloud.io/nitrux/testing/gpgkey | apt-key add -;

DEBIAN_FRONTEND=noninteractive apt -qq update

### Install Package Build Dependencies #2

DEBIAN_FRONTEND=noninteractive apt -qq -yy install --no-install-recommends \
	mauikit-git

### Download Source

git clone --depth 1 --branch $MAUISETTINGS_BRANCH https://github.com/Nitrux/maui-settings.git

### Compile Source

mkdir -p build && cd build

cmake \
	-DCMAKE_INSTALL_PREFIX=/usr \
	-DENABLE_BSYMBOLICFUNCTIONS=OFF \
	-DQUICK_COMPILER=ON \
	-DCMAKE_BUILD_TYPE=Release \
	-DCMAKE_INSTALL_SYSCONFDIR=/etc \
	-DCMAKE_INSTALL_LOCALSTATEDIR=/var \
	-DCMAKE_EXPORT_NO_PACKAGE_REGISTRY=ON \
	-DCMAKE_FIND_PACKAGE_NO_PACKAGE_REGISTRY=ON \
	-DCMAKE_INSTALL_RUNSTATEDIR=/run "-GUnix Makefiles" \
	-DCMAKE_VERBOSE_MAKEFILE=ON \
	-DCMAKE_INSTALL_LIBDIR=lib/x86_64-linux-gnu ../maui-settings/

make -j$(nproc)

### Run checkinstall and Build Debian Package

>> description-pak printf "%s\n" \
	'Maui Settings Manager is a settings manager for Maui Apps.' \
	'' \
	''

checkinstall -D -y \
	--install=no \
	--fstrans=yes \
	--pkgname=maui-settings-git \
	--pkgversion=$PACKAGE_VERSION \
	--pkgarch=amd64 \
	--pkgrelease="1" \
	--pkglicense=LGPL-3 \
	--pkggroup=utils \
	--pkgsource=maui-settings \
	--pakdir=. \
	--maintainer=uri_herrera@nxos.org \
	--provides=maui-settings \
	--requires="libc6,libqt5core5a,mauikit-git \(\>= 2.2.0+git\)" \
	--nodoc \
	--strip=no \
	--stripso=yes \
	--reset-uids=yes \
	--deldesc=yes
