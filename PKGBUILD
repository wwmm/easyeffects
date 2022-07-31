# Contributor: Wellington <wellingtonwallace@gmail.com>
# Maintainer: Upstream https://github.com/wwmm/easyeffects 

pkgname=easyeffects-git
pkgver=pkgvernotupdated
pkgrel=1
pkgdesc='Audio Effects for PipeWire Applications'
arch=(x86_64)
url='https://github.com/wwmm/easyeffects'
license=('GPL3')
depends=('libadwaita' 'pipewire-pulse' 'lilv' 'libsigc++-3.0' 'libsamplerate' 'zita-convolver' 
         'libebur128' 'rnnoise' 'rubberband' 'libbs2b' 'nlohmann-json' 'tbb' 'fmt')
makedepends=('meson' 'itstool' 'appstream-glib' 'git')
optdepends=('calf: limiter, exciter, bass enhancer and others'
            'lsp-plugins: equalizer, compressor, delay, loudness'
            'zam-plugins: maximizer'
            'mda.lv2: bass loudness'
            'yelp: in-app help')
conflicts=(easyeffects)
provides=(easyeffects)
replaces=('pulseeffects')
sha512sums=()

pkgver() {
  description=$(git describe --long | sed 's/^v//;s/\([^-]*-g\)/r\1/;s/-/./g')
  # if in github actions environment
  if test -f "../GITHUB_COMMIT_DESC"; then 
    # remove last commit from git describe output (which may sometimes be a merge commit),
    # and replace it with a human friendly version
    description_short=$(echo "$description" | sed -r 's/(.*)\..*/\1/')
    github_commit_desc_no_hyphen=$(sed 's/-/./g' ../GITHUB_COMMIT_DESC)
    printf "%s" "${description_short}.${github_commit_desc_no_hyphen}"
  else
    printf "%s" "$description"
  fi
}

build() {
  cd ..
  arch-meson . build

  ninja -C build
}

package() {
  cd ..
  DESTDIR="${pkgdir}" ninja install -C build
}
