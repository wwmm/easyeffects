# This PKGBUILD is intented for our github workflows. For building your custom packages use the file PKGBUILD_AUR

# Contributor: Wellington <wellingtonwallace@gmail.com>
# Maintainer: Upstream https://github.com/wwmm/easyeffects 

pkgname=easyeffects-git
pkgver=pkgvernotupdated
pkgrel=1
pkgdesc='Simple audio effects'
arch=(x86_64)
url='https://github.com/wwmm/easyeffects'
license=('GPL3')
depends=(
  'kcolorscheme' 
  'kirigami' 
  'kirigami-addons'
  'kiconthemes'
  'kconfigwidgets'
  'qqc2-desktop-style' 
  'breeze-icons' 
  'qt6-base'
  'qt6-graphs' 
  'qt6-webengine'
  'pipewire-pulse' 
  'lilv'
  'libsamplerate'
  'libsndfile' 
  'zita-convolver' 
  'libebur128' 
  'rnnoise' 
  'soundtouch' 
  'libbs2b' 
  'nlohmann-json' 
  'tbb' 
  'speexdsp'
  'gsl'
  'libportal-qt6'
  'webrtc-audio-processing'
)
makedepends=('cmake' 'extra-cmake-modules' 'git' 'ninja' 'intltool' 'appstream' 'mold' 'ladspa')
optdepends=('calf: limiter, exciter, bass enhancer and others'
            'lsp-plugins: equalizer, compressor, delay, loudness'
            'zam-plugins: maximizer'
            'mda.lv2: bass loudness'
            'libdeep_filter_ladspa: noise remover')
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

  cmake \
    -B build  \
    -S . \
    -G Ninja \
    -DCMAKE_INSTALL_PREFIX:PATH='/usr' \
    -Wno-dev

  cmake --build build
}

package() {
  cd ..
  DESTDIR="${pkgdir}" cmake --install build
}
