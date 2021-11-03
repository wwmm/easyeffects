itstool = find_program('itstool', required: true)

if not itstool.found()
  error('itstool not found.')
endif

help_pages = [
  'bassenhancer.page',
  'bassloudness.page',
  'compressor.page',
  'deesser.page',
  'exciter.page',
  'filter.page',
  'gate.page',
  'delay.page',
  'echocanceller.page',
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
  'crystalizer.page',
  'convolver.page',
  'settingsmenu.page',
  'general.page',
  'spectrum.page',
  'pipewire.page',
  'testsignals.page',
  'calibrationmic.page',
  'enableapp.page',
  'effectsorder.page',
  'userpresets.page',
  'advancedinfo.page',
  'saturated.page',
  'autogain.page',
  'blocklist.page',
  'guide_1.page',
  'rnnoise.page'
]

gnome_mod.yelp(meson.project_name(),
  sources: help_pages
)
