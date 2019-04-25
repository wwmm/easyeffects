schemadir = join_paths(datadir, 'glib-2.0', 'schemas')

install_data([
  'schemas/com.github.wwmm.pulseeffects.gschema.xml',
  'schemas/com.github.wwmm.pulseeffects.spectrum.gschema.xml',
  'schemas/com.github.wwmm.pulseeffects.sinkinputs.gschema.xml',
  'schemas/com.github.wwmm.pulseeffects.sinkinputs.autogain.gschema.xml',
  'schemas/com.github.wwmm.pulseeffects.sinkinputs.limiter.gschema.xml',
  'schemas/com.github.wwmm.pulseeffects.sinkinputs.compressor.gschema.xml',
  'schemas/com.github.wwmm.pulseeffects.sinkinputs.multibandcompressor.gschema.xml',
  'schemas/com.github.wwmm.pulseeffects.sinkinputs.reverb.gschema.xml',
  'schemas/com.github.wwmm.pulseeffects.sinkinputs.filter.gschema.xml',
  'schemas/com.github.wwmm.pulseeffects.sinkinputs.equalizer.gschema.xml',
  'schemas/com.github.wwmm.pulseeffects.sinkinputs.equalizer.leftchannel.gschema.xml',
  'schemas/com.github.wwmm.pulseeffects.sinkinputs.equalizer.rightchannel.gschema.xml',
  'schemas/com.github.wwmm.pulseeffects.sinkinputs.exciter.gschema.xml',
  'schemas/com.github.wwmm.pulseeffects.sinkinputs.bassenhancer.gschema.xml',
  'schemas/com.github.wwmm.pulseeffects.sinkinputs.crossfeed.gschema.xml',
  'schemas/com.github.wwmm.pulseeffects.sinkinputs.maximizer.gschema.xml',
  'schemas/com.github.wwmm.pulseeffects.sinkinputs.loudness.gschema.xml',
  'schemas/com.github.wwmm.pulseeffects.sinkinputs.pitch.gschema.xml',
  'schemas/com.github.wwmm.pulseeffects.sinkinputs.gate.gschema.xml',
  'schemas/com.github.wwmm.pulseeffects.sinkinputs.multibandgate.gschema.xml',
  'schemas/com.github.wwmm.pulseeffects.sinkinputs.stereotools.gschema.xml',
  'schemas/com.github.wwmm.pulseeffects.sinkinputs.deesser.gschema.xml',
  'schemas/com.github.wwmm.pulseeffects.sinkinputs.convolver.gschema.xml',
  'schemas/com.github.wwmm.pulseeffects.sinkinputs.crystalizer.gschema.xml',
  'schemas/com.github.wwmm.pulseeffects.sinkinputs.delay.gschema.xml',
  'schemas/com.github.wwmm.pulseeffects.sourceoutputs.gschema.xml',
  'schemas/com.github.wwmm.pulseeffects.sourceoutputs.limiter.gschema.xml',
  'schemas/com.github.wwmm.pulseeffects.sourceoutputs.compressor.gschema.xml',
  'schemas/com.github.wwmm.pulseeffects.sourceoutputs.multibandcompressor.gschema.xml',
  'schemas/com.github.wwmm.pulseeffects.sourceoutputs.reverb.gschema.xml',
  'schemas/com.github.wwmm.pulseeffects.sourceoutputs.filter.gschema.xml',
  'schemas/com.github.wwmm.pulseeffects.sourceoutputs.equalizer.gschema.xml',
  'schemas/com.github.wwmm.pulseeffects.sourceoutputs.equalizer.leftchannel.gschema.xml',
  'schemas/com.github.wwmm.pulseeffects.sourceoutputs.equalizer.rightchannel.gschema.xml',
  'schemas/com.github.wwmm.pulseeffects.sourceoutputs.pitch.gschema.xml',
  'schemas/com.github.wwmm.pulseeffects.sourceoutputs.gate.gschema.xml',
  'schemas/com.github.wwmm.pulseeffects.sourceoutputs.deesser.gschema.xml',
  'schemas/com.github.wwmm.pulseeffects.sourceoutputs.webrtc.gschema.xml',
  'schemas/com.github.wwmm.pulseeffects.sourceoutputs.multibandgate.gschema.xml'
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
	'pulseeffects-resources',
	'pulseeffects.gresource.xml',
	c_name: meson.project_name(),
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
