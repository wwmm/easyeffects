# Contributor: Wellington <wellingtonwallace@gmail.com>

pkgname=easyeffects-git
pkgver=6.0.2.r21.g4b5a60ab
pkgrel=1
pkgdesc='Audio Effects for PipeWire Applications'
arch=(x86_64)
url='https://github.com/wwmm/easyeffects'
license=('GPL3')
depends=('gtkmm-4.0' 'glibmm-2.68' 'pipewire' 'lilv' 'lv2' 'libsigc++-3.0' 'libsndfile' 'libsamplerate' 'zita-convolver' 
         'libebur128' 'rnnoise' 'rubberband' 'fftw' 'libbs2b' 'speexdsp' 'nlohmann-json')
makedepends=('meson' 'itstool' 'appstream-glib')
optdepends=('calf: limiter, exciter, bass enhancer and others'
            'lsp-plugins: equalizer, compressor, delay, loudness'
            'zam-plugins: maximizer'
            'mda.lv2: bass loudness'
            'yelp: in-app help')
source=("easyeffects::git+https://github.com/wwmm/easyeffects.git")
conflicts=(easyeffects)
provides=(easyeffects)
replaces=('pulseeffects')
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
