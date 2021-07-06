# Contributor: Wellington <wellingtonwallace@gmail.com>

pkgname=easyeffects-git
pkgver=5.0.1.r544.gc1d3c52a
pkgrel=1
pkgdesc='Audio Effects for Pulseaudio Applications'
arch=(x86_64)
url='https://github.com/wwmm/easyeffects'
license=('GPL3')
depends=('gtkmm-4.0' 'glibmm-2.68' 'pipewire' 'lilv' 'lv2' 'libsigc++-3.0' 'libsndfile' 'libsamplerate' 'zita-convolver' 
         'libebur128' 'rnnoise' 'rubberband' 'fftw' 'libbs2b' 'speexdsp' 'nlohmann-json')
makedepends=('meson' 'itstool' 'appstream-glib')
optdepends=('calf: limiter, compressor exciter, bass enhancer and others'
            'lsp-plugins: equalizer, delay'
            'zam-plugins: maximizer'
            'yelp: in-app help')
source=("easyeffects::git+https://github.com/wwmm/pulseeffects.git")
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
