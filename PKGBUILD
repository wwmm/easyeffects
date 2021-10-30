# Contributor: Wellington <wellingtonwallace@gmail.com>

pkgname=easyeffects-git
pkgver=6.1.4.r19.gc63c5dd3
pkgrel=1
pkgdesc='Audio Effects for PipeWire Applications'
arch=(x86_64)
url='https://github.com/wwmm/easyeffects'
license=('GPL3')
depends=('gtkmm-4.0' 'glibmm-2.68' 'pipewire' 'lilv' 'lv2' 'libsigc++-3.0' 'libsndfile' 'libsamplerate' 'zita-convolver' 
         'libebur128' 'rnnoise' 'rubberband' 'fftw' 'libbs2b' 'speexdsp' 'nlohmann-json' 'tbb')
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
  arch-meson easyeffects build

  ninja -C build
}

package() {
  DESTDIR="${pkgdir}" ninja install -C build
}
