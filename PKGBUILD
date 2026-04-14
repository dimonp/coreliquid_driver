pkgname=msi-meg-coreliquid-s360-driver
pkgver=0.1.3
pkgrel=1
pkgdesc="Driver MSI Coreliquid MEG S360 (GPL3, hidapi)"
arch=('x86_64')
url="https://github.com/dimonp/coreliquid_driver"
license=('GPL3')
depends=('hidapi' 'glibc' 'dbus')
makedepends=('cmake' 'gcc')
install=msi-coreliquid-driver.install
options=('!debug')

source=(
    "99-msi-coreliquid.rules"
    "10-msi-coreliquid.rules"
    "io.github.MSICoreliquid.conf"
)
sha256sums=(
    'SKIP'
    'SKIP'
    'SKIP'
)

build() {
    cmake -B build -S "$startdir" \
        -DPROJECT_VERSION="$pkgver" \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX=/usr/local

    cmake --build build --config Release
}

package() {
    DESTDIR="${pkgdir}" cmake --install build

    # udev rules
    install -Dm644 "${srcdir}/99-msi-coreliquid.rules" "${pkgdir}/usr/lib/udev/rules.d/99-msi-coreliquid.rules"
    # polkit rules
    install -Dm644 "${srcdir}/10-msi-coreliquid.rules" "${pkgdir}/usr/share/polkit-1/rules.d/10-msi-coreliquid.rules"
    # dbus conf
    install -Dm644 "${srcdir}/io.github.MSICoreliquid.conf" "${pkgdir}/etc/dbus-1/system.d/io.github.MSICoreliquid.conf"

    if command -v kpackagetool6 >/dev/null 2>&1; then
        echo "  -> Plasma 6 environment detected. Installing widget..."

        local _widget_id="io.github.msicoreliquid.switcher"
        local _dest="$pkgdir/usr/share/plasma/plasmoids/$_widget_id"

        install -d "$_dest"
        cp -r "$startdir/widget/"* "$_dest/"
    else
        echo "  -> kpackagetool6 not found. Skipping Plasma widget installation."
    fi
}
