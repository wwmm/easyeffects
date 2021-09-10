schemadir = join_paths(datadir, 'glib-2.0', 'schemas')

install_data([
  'schemas/com.github.wwmm.easyeffects.gschema.xml',
  'schemas/com.github.wwmm.easyeffects.autogain.gschema.xml',
  'schemas/com.github.wwmm.easyeffects.bassenhancer.gschema.xml',
  'schemas/com.github.wwmm.easyeffects.bassloudness.gschema.xml',
  'schemas/com.github.wwmm.easyeffects.compressor.gschema.xml',
  'schemas/com.github.wwmm.easyeffects.convolver.gschema.xml',
  'schemas/com.github.wwmm.easyeffects.crossfeed.gschema.xml',
  'schemas/com.github.wwmm.easyeffects.crystalizer.gschema.xml',
  'schemas/com.github.wwmm.easyeffects.deesser.gschema.xml',
  'schemas/com.github.wwmm.easyeffects.delay.gschema.xml',
  'schemas/com.github.wwmm.easyeffects.echo_canceller.gschema.xml',
  'schemas/com.github.wwmm.easyeffects.equalizer.gschema.xml',
  'schemas/com.github.wwmm.easyeffects.equalizer.channel.gschema.xml',
  'schemas/com.github.wwmm.easyeffects.exciter.gschema.xml',
  'schemas/com.github.wwmm.easyeffects.filter.gschema.xml',
  'schemas/com.github.wwmm.easyeffects.gate.gschema.xml',
  'schemas/com.github.wwmm.easyeffects.limiter.gschema.xml',
  'schemas/com.github.wwmm.easyeffects.loudness.gschema.xml',
  'schemas/com.github.wwmm.easyeffects.maximizer.gschema.xml',
  'schemas/com.github.wwmm.easyeffects.multibandcompressor.gschema.xml',
  'schemas/com.github.wwmm.easyeffects.multibandgate.gschema.xml',
  'schemas/com.github.wwmm.easyeffects.outputlevel.gschema.xml',
  'schemas/com.github.wwmm.easyeffects.pitch.gschema.xml',
  'schemas/com.github.wwmm.easyeffects.reverb.gschema.xml',
  'schemas/com.github.wwmm.easyeffects.rnnoise.gschema.xml',
  'schemas/com.github.wwmm.easyeffects.spectrum.gschema.xml',
  'schemas/com.github.wwmm.easyeffects.streamoutputs.gschema.xml',
  'schemas/com.github.wwmm.easyeffects.stereotools.gschema.xml',
  'schemas/com.github.wwmm.easyeffects.streaminputs.gschema.xml'
], install_dir: schemadir)

metainfo_file = i18n_mod.merge_file(
  input: 'com.github.wwmm.easyeffects.metainfo.xml.in',
  output: 'com.github.wwmm.easyeffects.metainfo.xml',
  po_dir: '../po',
  install: true,
  install_dir: join_paths(datadir, 'metainfo')
)

# Validate merged AppStream metainfo file
appstream_util = find_program('appstream-util', required: false)
if appstream_util.found()
  test('Validate appstream file', appstream_util,
    args: ['validate-strict', '--nonet', metainfo_file]
  )
endif

desktop_file = i18n_mod.merge_file(
  input: 'com.github.wwmm.easyeffects.desktop.in',
  output: 'com.github.wwmm.easyeffects.desktop',
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
install_data('easyeffects.svg',
  install_dir: join_paths(icondir, 'scalable', 'apps')
)

conf_glade = configuration_data()
conf_glade.set('PROJECT_VERSION', meson.project_version())

gresources = gnome_mod.compile_resources(
	'easyeffects-resources',
	'easyeffects.gresource.xml',
	c_name: meson.project_name()
)

serviceconf = configuration_data()
serviceconf.set('bindir', join_paths(get_option('prefix'), get_option('bindir')))

configure_file(
  input: 'com.github.wwmm.easyeffects.service.in',
  output: 'com.github.wwmm.easyeffects.service',
  configuration: serviceconf,
  install_dir: join_paths(get_option('datadir'), 'dbus-1', 'services'),
)

configure_file(
  input: 'easyeffects.service.in',
  output: 'easyeffects.service',
  configuration: serviceconf,
  install_dir: join_paths(get_option('libdir'), 'systemd', 'user'),
)
