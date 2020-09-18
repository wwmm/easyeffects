# Maintainer: Bleuzen <supgesu@gmail.com>
# Contributor: Filipe Laíns (FFY00) <lains@archlinux.org>
# Contributor: Wellington <wellingtonwallace@gmail.com>

pkgname=pulseeffects-git
pkgver=4.7.1.r2.g38355f59
pkgrel=1
pkgdesc='Audio Effects for Pulseaudio Applications'
arch=(x86_64 i686 arm armv6h armv7h aarch64)
url='https://github.com/wwmm/pulseeffects'
license=('GPL3')
depends=('gtk3' 'gtkmm3' 'glibmm' 'libpulse' 'gstreamer' 'gst-plugins-good' 'gst-plugins-bad'
        'lilv' 'boost-libs' 'libsigc++' 'libsndfile' 'libsamplerate' 'zita-convolver' 'libebur128')
makedepends=('meson' 'boost' 'itstool' 'appstream-glib' 'calf' 'zam-plugins' 'rubberband')
optdepends=('calf: limiter, compressor exciter, bass enhancer and others'
            'lsp-plugins: equalizer, delay'
            'rubberband: pitch shifting'
            'zam-plugins: maximizer'
            'yelp: in-app help')
source=("git+https://github.com/wwmm/pulseeffects.git")
conflicts=(pulseeffects)
provides=(pulseeffects)
sha512sums=('SKIP')

pkgver() {
  cd pulseeffects
  git describe --long | sed 's/^v//;s/\([^-]*-g\)/r\1/;s/-/./g'
}

build() {
  mkdir -p pulseeffects/build
  cd pulseeffects/build

  arch-meson ..

  ninja
}

package() {
  cd pulseeffects/build

  DESTDIR="$pkgdir" ninja install
}
