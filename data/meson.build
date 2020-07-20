schemadir = join_paths(datadir, 'glib-2.0', 'schemas')

install_data([
  'schemas/com.github.wwmm.pulseeffects.gschema.xml',
  'schemas/com.github.wwmm.pulseeffects.spectrum.gschema.xml',
  'schemas/com.github.wwmm.pulseeffects.sinkinputs.gschema.xml',
  'schemas/com.github.wwmm.pulseeffects.autogain.gschema.xml',
  'schemas/com.github.wwmm.pulseeffects.limiter.gschema.xml',
  'schemas/com.github.wwmm.pulseeffects.compressor.gschema.xml',
  'schemas/com.github.wwmm.pulseeffects.multibandcompressor.gschema.xml',
  'schemas/com.github.wwmm.pulseeffects.reverb.gschema.xml',
  'schemas/com.github.wwmm.pulseeffects.filter.gschema.xml',
  'schemas/com.github.wwmm.pulseeffects.equalizer.gschema.xml',
  'schemas/com.github.wwmm.pulseeffects.equalizer.channel.gschema.xml',
  'schemas/com.github.wwmm.pulseeffects.exciter.gschema.xml',
  'schemas/com.github.wwmm.pulseeffects.bassenhancer.gschema.xml',
  'schemas/com.github.wwmm.pulseeffects.crossfeed.gschema.xml',
  'schemas/com.github.wwmm.pulseeffects.maximizer.gschema.xml',
  'schemas/com.github.wwmm.pulseeffects.loudness.gschema.xml',
  'schemas/com.github.wwmm.pulseeffects.pitch.gschema.xml',
  'schemas/com.github.wwmm.pulseeffects.gate.gschema.xml',
  'schemas/com.github.wwmm.pulseeffects.multibandgate.gschema.xml',
  'schemas/com.github.wwmm.pulseeffects.stereotools.gschema.xml',
  'schemas/com.github.wwmm.pulseeffects.deesser.gschema.xml',
  'schemas/com.github.wwmm.pulseeffects.convolver.gschema.xml',
  'schemas/com.github.wwmm.pulseeffects.crystalizer.gschema.xml',
  'schemas/com.github.wwmm.pulseeffects.delay.gschema.xml',
  'schemas/com.github.wwmm.pulseeffects.sourceoutputs.gschema.xml',
  'schemas/com.github.wwmm.pulseeffects.webrtc.gschema.xml'
], install_dir: schemadir)

appdata_file = i18n_mod.merge_file(
  input: 'com.github.wwmm.pulseeffects.appdata.xml.in',
  output: 'com.github.wwmm.pulseeffects.appdata.xml',
  po_dir: '../po',
  install: true,
  install_dir: join_paths(datadir, 'metainfo')
)

# Validate merged AppStream appdata file
appstream_util = find_program('appstream-util', required: false)
if appstream_util.found()
  test('Validate appstream file', appstream_util,
    args: ['validate-strict', '--nonet', appdata_file]
  )
endif

desktop_file = i18n_mod.merge_file(
  input: 'com.github.wwmm.pulseeffects.desktop.in',
  output: 'com.github.wwmm.pulseeffects.desktop',
  type: 'desktop',
  po_dir: '../po',
  install: true,
  install_dir: join_paths(datadir, 'applications')
)

# Validate merged desktop entry file
desktop_utils = find_program('desktop-file-validate', required: false)
if desktop_utils.found()
  test('Validate desktop file', desktop_utils,
    args: [desktop_file]
  )
endif

icondir = join_paths(datadir, 'icons', 'hicolor')
install_data('pulseeffects.svg',
  install_dir: join_paths(icondir, 'scalable', 'apps')
)

conf_glade = configuration_data()
conf_glade.set('PROJECT_VERSION', meson.project_version())

glade_about = configure_file(input: 'ui/about.glade.in',
               output: 'about.glade',
               configuration: conf_glade)

gresources = gnome_mod.compile_resources(
	'resources',
	'pulseeffects.gresource.xml',
	c_name: meson.project_name(),
  gresource_bundle: true,
  dependencies: [glade_about]
)

dbusconf = configuration_data()
dbusconf.set('bindir', join_paths(get_option('prefix'), get_option('bindir')))

configure_file(
  input: 'com.github.wwmm.pulseeffects.service.in',
  output: 'com.github.wwmm.pulseeffects.service',
  configuration: dbusconf,
  install_dir: join_paths(get_option('datadir'), 'dbus-1', 'services'),
)
