# Maintainer: Bleuzen <supgesu@gmail.com>
# Contributor: Filipe Laíns (FFY00) <lains@archlinux.org>
# Contributor: Wellington <wellingtonwallace@gmail.com>

pkgname=easyeffects-git
pkgver=4.8.4.r56.g52dbcbc6
pkgrel=1
pkgdesc='Audio Effects for Pulseaudio Applications'
arch=(x86_64 i686 arm armv6h armv7h aarch64)
url='https://github.com/wwmm/easyeffects'
license=('GPL3')
depends=('gtk3' 'gtkmm3' 'glibmm' 'pipewire' 'gstreamer' 'gst-plugin-pipewire' 'gst-plugins-good' 'gst-plugins-bad'
        'lilv' 'boost-libs' 'libsigc++' 'libsndfile' 'libsamplerate' 'zita-convolver' 'libebur128')
makedepends=('meson' 'boost' 'itstool' 'appstream-glib' 'calf' 'zam-plugins' 'rubberband' 'librnnoise')
optdepends=('calf: limiter, compressor exciter, bass enhancer and others'
            'lsp-plugins: equalizer, delay'
            'rubberband: pitch shifting'
            'zam-plugins: maximizer'
            'librnnoise: noise remover'
            'yelp: in-app help')
source=("git+https://github.com/wwmm/easyeffects.git")
conflicts=(easyeffects)
provides=(easyeffects)
sha512sums=('SKIP')

pkgver() {
  cd easyeffects
  git describe --long | sed 's/^v//;s/\([^-]*-g\)/r\1/;s/-/./g'
}

build() {
  mkdir -p easyeffects/build
  cd easyeffects/build

  arch-meson ..

  ninja
}

package() {
  cd easyeffects/build

  DESTDIR="$pkgdir" ninja install
}
