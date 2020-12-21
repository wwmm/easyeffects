# Maintainer: Bleuzen <supgesu@gmail.com>
# Contributor: Filipe Laíns (FFY00) <lains@archlinux.org>
# Contributor: Wellington <wellingtonwallace@gmail.com>

pkgname=pulseeffects-legacy-git
pkgver=4.7.1.r2.g38355f59
pkgrel=1
pkgdesc='Audio Effects for Pulseaudio Applications'
arch=(x86_64)
url='https://github.com/wwmm/pulseeffects'
license=('GPL3')
depends=('gtk3' 'gtkmm3' 'glibmm' 'pulseaudio'
         'gstreamer' 'gst-plugin-gtk' 'gst-plugins-bad'
         'lilv' 'boost-libs' 'libsigc++' 'libsndfile'
         'libsamplerate' 'zita-convolver' 'libebur128'
         'calf' 'lsp-plugins' 'yelp')
makedepends=('meson' 'boost' 'itstool' 'appstream-glib'
             'zam-plugins' 'rubberband' 'librnnoise')
optdepends=('zam-plugins: maximizer'
            'rubberband: pitch shifting'
            'librnnoise: noise remover')
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
