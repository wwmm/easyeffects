itstool = find_program('itstool', required: true)

if not itstool.found()
  error('itstool not found.')
endif

help_pages = [
  'bassenhancer.page',
  'compressor.page',
  'deesser.page',
  'exciter.page',
  'filter.page',
  'gate.page',
  'delay.page',
  'index.page',
  'limiter.page',
  'loudness.page',
  'multibandcompressor.page',
  'multibandgate.page',
  'reverb.page',
  'stereotools.page',
  'maximizer.page',
  'crossfeed.page',
  'equalizer.page',
  'pitch.page',
  'webrtc.page',
  'crystalizer.page',
  'convolver.page',
  'settingsmenu.page',
  'general.page',
  'spectrum.page',
  'pulseaudio.page',
  'testsignals.page',
  'calibrationmic.page',
  'enableapp.page',
  'effectsorder.page',
  'userpresets.page',
  'advancedinfo.page',
  'saturated.page',
  'autogain.page',
  'blacklist.page'
]

help_media = [
  'figures/effectsorder.png',
  'figures/enableapp.png',
  'figures/settingsmenu.png',
  'figures/userpresets.png',
  'figures/advancedinfo.png',
  'figures/saturated.png',
  'figures/blacklist.png'
]

gnome_mod.yelp(meson.project_name(),
  sources: help_pages,
  media: help_media
)
