# Maintainer: robertfoster
# Contributor: Bleuzen <supgesu@gmail.com>
# Contributor: Filipe Laíns (FFY00) <lains@archlinux.org>
# Contributor: Wellington <wellingtonwallace@gmail.com>

pkgname=easyeffects-git
pkgver=7.0.0.r338.g92fe5ed5
pkgrel=1
pkgdesc='Audio Effects for Pipewire applications'
arch=(x86_64 i686 arm armv6h armv7h aarch64)
url='https://github.com/wwmm/easyeffects'
license=('GPL3')
depends=('fftw' 'fmt' 'gsl' 'gtk4' 'libadwaita' 'libbs2b' 'libebur128' 'libsamplerate' 'libsigc++-3.0' 'libsndfile'
  'lilv' 'lv2' 'nlohmann-json' 'pipewire' 'rnnoise' 'rubberband' 'speexdsp' 'tbb' 'zita-convolver')
makedepends=('appstream-glib' 'git' 'itstool' 'meson')
optdepends=('calf: limiter, exciter, bass enhancer and others'
  'lsp-plugins: equalizer, compressor, delay, loudness'
  'zam-plugins: maximizer'
  'mda.lv2: bass loudness'
  'yelp: in-app help')
conflicts=("${pkgname%%-git}")
provides=("${pkgname%%-git}")
source=("${pkgname%%-git}::git+${url}")
sha512sums=('SKIP')

pkgver() {
  cd "${pkgname%%-git}"
  printf "%s" "$(git describe --long | sed 's/^v//;s/\([^-]*-g\)/r\1/;s/-/./g')"
}

build() {
  arch-meson "${pkgname%%-git}" build
  meson compile -C build
}

package() {
  DESTDIR="${pkgdir}" meson install -C build
}
