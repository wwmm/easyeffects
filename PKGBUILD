# Maintainer: Bleuzen <supgesu@gmail.com>
# Contributor: Filipe Laíns (FFY00) <lains@archlinux.org>
# Contributor: Wellington <wellingtonwallace@gmail.com>


pkgname=pulseeffects-legacy-git
pkgver=4.8.4.r0.gcef23707
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
source=("git+https://github.com/Digitalone1/pulseeffects-1.git")
conflicts=(pulseeffects)
provides=(pulseeffects)
sha512sums=('SKIP')

pkgver() {
  cd pulseeffects-1
  git describe --long | sed 's/^v//;s/\([^-]*-g\)/r\1/;s/-/./g'
}

build() {
  mkdir -p pulseeffects-1/build
  cd pulseeffects-1/build

  arch-meson ..

  ninja
}

package() {
  cd pulseeffects-1/build

  DESTDIR="$pkgdir" ninja install
}
