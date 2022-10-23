itstool = find_program('itstool', required: true)

if not itstool.found()
  error('itstool not found.')
endif

help_pages = [
  'advancedinfo.page',
  'autogain.page',
  'bassenhancer.page',
  'bassloudness.page',
  'blocklist.page',
  'compressor.page',
  'convolver.page',
  'crossfeed.page',
  'crystalizer.page',
  'deesser.page',
  'delay.page',
  'echocanceller.page',
  'effectsorder.page',
  'enableapp.page',
  'equalizer.page',
  'exciter.page',
  'filter.page',
  'gate.page',
  'general.page',
  'guide_1.page',
  'index.page',
  'limiter.page',
  'loudness.page',
  'maximizer.page',
  'multibandcompressor.page',
  'multibandgate.page',
  'pipewire.page',
  'pitch.page',
  'reverb.page',
  'rnnoise.page',
  'saturated.page',
  'settingsmenu.page',
  'spectrum.page',
  'speex.page',
  'stereotools.page',
  'testsignals.page',
  'userpresets.page'
]

gnome_mod.yelp(meson.project_name(),
  sources: help_pages
)
