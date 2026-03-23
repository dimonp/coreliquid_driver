pkgname=msi-meg-coreliquid-s360-driver
pkgver=0.1.0
pkgrel=1
pkgdesc="Driver MSI Coreliquid MEG S360 (GPL3, hidapi)"
arch=('x86_64')
url="https://github.com/dimonp/coreliquid_driver"
license=('GPL3')
depends=('hidapi' 'glibc')
makedepends=('cmake' 'gcc')
install=msi-coreliquid-driver.install
options=('!debug')

source=("99-msi-coreliquid.rules")
sha256sums=('SKIP')

build() {
    cmake -B build -S "$startdir" \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX=/usr/local

    cmake --build build --config Release
}

package() {
    DESTDIR="${pkgdir}" cmake --install build

    install -Dm644 "${srcdir}/99-msi-coreliquid.rules" "${pkgdir}/usr/lib/udev/rules.d/99-msi-coreliquid.rules"
}
