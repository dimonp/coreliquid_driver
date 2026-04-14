pkgname=msi-meg-coreliquid-s360-driver
pkgname=('msi-meg-coreliquid-s360-driver' 'msi-meg-coreliquid-plasma-widget')
pkgver=0.1.4
pkgrel=1
arch=('x86_64')
url="https://github.com/dimonp/coreliquid_driver"
license=('GPL3')
makedepends=('cmake' 'gcc')
options=('!debug')

source=(
    "99-msi-coreliquid.rules"
    "10-msi-coreliquid.rules"
    "io.github.MSICoreliquid.conf"
)
sha256sums=('SKIP' 'SKIP' 'SKIP')

build() {
    cmake -B ${srcdir}/build -S "$startdir" \
        -DPROJECT_VERSION="$pkgver" \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX=/usr \
        -DUSE_SYSTEMD_BUS=ON

    cmake --build ${srcdir}/build
}

# Driver package
package_msi-meg-coreliquid-s360-driver() {
    pkgdesc="Driver MSI Coreliquid MEG S360 (GPL3, hidapi)"
    depends=('hidapi' 'glibc' 'dbus')
    install=msi-coreliquid-driver.install

    DESTDIR="${pkgdir}" cmake --install build

    install -Dm644 "${srcdir}/99-msi-coreliquid.rules" "${pkgdir}/usr/lib/udev/rules.d/99-msi-coreliquid.rules"
    install -Dm644 "${srcdir}/10-msi-coreliquid.rules" "${pkgdir}/usr/share/polkit-1/rules.d/10-msi-coreliquid.rules"
    install -Dm644 "${srcdir}/io.github.MSICoreliquid.conf" "${pkgdir}/etc/dbus-1/system.d/io.github.MSICoreliquid.conf"
}

package_msi-meg-coreliquid-plasma-widget() {
    pkgdesc="Plasma 6 widget for MSI Coreliquid MEG S360"
    depends=("$pkgbase" 'plasma-workspace')
    install=msi-coreliquid-widget.install

    local _widget_id="io.github.msicoreliquid.switcher"
    local _dest="$pkgdir/usr/share/plasma/plasmoids/$_widget_id"

    install -d "$_dest"
    cp -r "$startdir/widget/"* "$_dest/"
}
