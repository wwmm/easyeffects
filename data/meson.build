schemadir = join_paths(datadir, 'glib-2.0', 'schemas')

install_data([
  'schemas/com.github.wwmm.pulseeffects.gschema.xml',
  'schemas/com.github.wwmm.pulseeffects.sinkinputs.gschema.xml',
  'schemas/com.github.wwmm.pulseeffects.sinkinputs.limiter.gschema.xml',
  'schemas/com.github.wwmm.pulseeffects.sinkinputs.compressor.gschema.xml',
  'schemas/com.github.wwmm.pulseeffects.sinkinputs.multibandcompressor.gschema.xml',
  'schemas/com.github.wwmm.pulseeffects.sinkinputs.reverb.gschema.xml',
  'schemas/com.github.wwmm.pulseeffects.sinkinputs.filter.gschema.xml',
  'schemas/com.github.wwmm.pulseeffects.sinkinputs.equalizer.gschema.xml',
  'schemas/com.github.wwmm.pulseeffects.sinkinputs.exciter.gschema.xml',
  'schemas/com.github.wwmm.pulseeffects.sinkinputs.bassenhancer.gschema.xml',
  'schemas/com.github.wwmm.pulseeffects.sinkinputs.stereoenhancer.gschema.xml',
  'schemas/com.github.wwmm.pulseeffects.sinkinputs.crossfeed.gschema.xml',
  'schemas/com.github.wwmm.pulseeffects.sinkinputs.maximizer.gschema.xml',
  'schemas/com.github.wwmm.pulseeffects.sinkinputs.loudness.gschema.xml',
  'schemas/com.github.wwmm.pulseeffects.sinkinputs.gate.gschema.xml',
  'schemas/com.github.wwmm.pulseeffects.sinkinputs.multibandgate.gschema.xml',
  'schemas/com.github.wwmm.pulseeffects.sinkinputs.stereotools.gschema.xml',
  'schemas/com.github.wwmm.pulseeffects.sinkinputs.deesser.gschema.xml',
  'schemas/com.github.wwmm.pulseeffects.sinkinputs.convolver.gschema.xml',
  'schemas/com.github.wwmm.pulseeffects.sinkinputs.crystalizer.gschema.xml',
  'schemas/com.github.wwmm.pulseeffects.sourceoutputs.gschema.xml',
  'schemas/com.github.wwmm.pulseeffects.sourceoutputs.limiter.gschema.xml',
  'schemas/com.github.wwmm.pulseeffects.sourceoutputs.compressor.gschema.xml',
  'schemas/com.github.wwmm.pulseeffects.sourceoutputs.multibandcompressor.gschema.xml',
  'schemas/com.github.wwmm.pulseeffects.sourceoutputs.reverb.gschema.xml',
  'schemas/com.github.wwmm.pulseeffects.sourceoutputs.filter.gschema.xml',
  'schemas/com.github.wwmm.pulseeffects.sourceoutputs.equalizer.gschema.xml',
  'schemas/com.github.wwmm.pulseeffects.sourceoutputs.pitch.gschema.xml',
  'schemas/com.github.wwmm.pulseeffects.sourceoutputs.gate.gschema.xml',
  'schemas/com.github.wwmm.pulseeffects.sourceoutputs.deesser.gschema.xml',
  'schemas/com.github.wwmm.pulseeffects.sourceoutputs.webrtc.gschema.xml',
  'schemas/com.github.wwmm.pulseeffects.sourceoutputs.multibandgate.gschema.xml'
], install_dir: schemadir)

i18n_mod.merge_file(
  input: 'com.github.wwmm.pulseeffects.appdata.xml.in',
  output: 'com.github.wwmm.pulseeffects.appdata.xml',
  po_dir: '../po',
  install: true,
  install_dir: join_paths(datadir, 'metainfo')
)

i18n_mod.merge_file(
  input: 'com.github.wwmm.pulseeffects.desktop.in',
  output: 'com.github.wwmm.pulseeffects.desktop',
  type: 'desktop',
  po_dir: '../po',
  install: true,
  install_dir: join_paths(datadir, 'applications')
)

icondir = join_paths(datadir, 'icons', 'hicolor')
install_data('pulseeffects.svg',
  install_dir: join_paths(icondir, 'scalable', 'apps')
)

gresources = gnome_mod.compile_resources(
	'pulseeffects-resources',
	'pulseeffects.gresource.xml',
	c_name: meson.project_name()
)
