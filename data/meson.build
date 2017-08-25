schemadir = join_paths(get_option('datadir'), 'glib-2.0/schemas')
install_data([
  'com.github.wwmm.pulseeffects.gschema.xml',
  'com.github.wwmm.pulseeffects.sinkinputs.gschema.xml',
  'com.github.wwmm.pulseeffects.sourceoutputs.gschema.xml',
], install_dir: schemadir)

menudir = join_paths(get_option('datadir'), 'applications')
install_data([
  'com.github.wwmm.pulseeffects.desktop'
], install_dir: menudir)

icondir = join_paths(get_option('datadir'), 'icons/hicolor')
install_data('pulseeffects.svg',
  install_dir: join_paths(icondir, 'scalable/apps')
)
