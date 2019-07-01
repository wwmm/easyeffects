# Maintainer: Wellington <wellingtonwallace@gmail.com>
_pkgname=pulseeffects
pkgname="${_pkgname}-git"
pkgver=v4.1.9.r53.g24d322a
pkgrel=1
pkgdesc="Audio Effects for Pulseaudio Applications"
arch=(any)
url="https://github.com/wwmm/pulseeffects"
license=('GPL3')
depends=(gtk3 gtkmm3 glibmm libpulse gstreamer gst-plugins-good gst-plugins-bad
        lilv boost-libs libsigc++ libsndfile libsamplerate zita-convolver
        libebur128)
optdepends=('calf: limiter, multiband compressor, exciter, bass enhancer and others'
            'zam-plugins: maximizer'
            'rubberband: pitch shifting'
            'lsp-plugins: equalizer, sidechain compressor and delay'
            'mda.lv2: loudness'
            'yelp: documentation')
makedepends=('meson' 'boost' 'itstool')
options=(!emptydirs)
provides=("${_pkgname}")
conflicts=("${_pkgname}")
source=("${_pkgname}::git+https://github.com/wwmm/pulseeffects.git")
sha256sums=('SKIP')

pkgver() {
    cd "${srcdir}/${_pkgname}"
    git describe --long | sed 's/\([^-]*-g\)/r\1/;s/-/./g'
}

build() {
  cd "${srcdir}/${_pkgname}"
  # Remove any potential residual build files
  rm -rf _build
  meson _build --prefix=/usr --buildtype=release
}

package() {
  cd "${srcdir}/${_pkgname}/_build"
  env DESTDIR="$pkgdir" ninja install
}

# vim:set ts=2 sw=2 et:
