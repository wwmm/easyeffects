schemadir = join_paths(datadir, 'glib-2.0', 'schemas')
install_data([
  'com.github.wwmm.pulseeffects.gschema.xml',
  'com.github.wwmm.pulseeffects.sinkinputs.gschema.xml',
  'com.github.wwmm.pulseeffects.sinkinputs.limiter.gschema.xml',
  'com.github.wwmm.pulseeffects.sinkinputs.panorama.gschema.xml',
  'com.github.wwmm.pulseeffects.sinkinputs.compressor.gschema.xml',
  'com.github.wwmm.pulseeffects.sourceoutputs.gschema.xml',
  'com.github.wwmm.pulseeffects.sourceoutputs.limiter.gschema.xml',
  'com.github.wwmm.pulseeffects.sourceoutputs.compressor.gschema.xml',
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
