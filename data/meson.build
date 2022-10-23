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
  'schemas/com.github.wwmm.easyeffects.speex.gschema.xml',
  'schemas/com.github.wwmm.easyeffects.streamoutputs.gschema.xml',
  'schemas/com.github.wwmm.easyeffects.stereotools.gschema.xml',
  'schemas/com.github.wwmm.easyeffects.streaminputs.gschema.xml'
], install_dir: schemadir)

if get_option('enable-libportal') and libportal.found()
  install_data([
    'schemas/com.github.wwmm.easyeffects.libportal.gschema.xml',
    'schemas/overrides/com.github.wwmm.easyeffects.gschema.override'
], install_dir: schemadir)
endif

data_conf = configuration_data()

data_conf.set('BIN_DIR', bindir)

data_conf.set('APP_ID', app_id)
data_conf.set('APP_NAME', 'Easy Effects' + name_suffix)

metainfo_file = i18n_mod.merge_file(
  input: configure_file(
    input: files('com.github.wwmm.easyeffects.metainfo.xml.in'),
    output: 'com.github.wwmm.easyeffects.metainfo.xml.in',
    configuration: data_conf
  ),
  output: '@0@.metainfo.xml'.format(app_id),
  po_dir: '../po',
  install: true,
  install_dir: join_paths(datadir, 'metainfo')
)

# Validate merged AppStream metainfo file
appstream_util = find_program('appstream-util', required: false)
if appstream_util.found()
  test('Validate appstream file', appstream_util,
    args: ['validate-relax', '--nonet', metainfo_file]
  )
endif

desktop_file = i18n_mod.merge_file(
  input: configure_file(
    input: files('com.github.wwmm.easyeffects.desktop.in'),
    output: 'com.github.wwmm.easyeffects.desktop.in',
    configuration: data_conf
  ),
  output: '@0@.desktop'.format(app_id),
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
install_data('@0@.svg'.format(app_id),
  install_dir: join_paths(icondir, 'scalable', 'apps'),
  rename: '@0@.svg'.format(app_id)
)

install_data('com.github.wwmm.easyeffects-symbolic.svg',
  install_dir: join_paths(icondir, 'symbolic', 'apps'),
  rename: '@0@-symbolic.svg'.format(app_id)
)


gresources = gnome_mod.compile_resources(
	'easyeffects-resources',
	'easyeffects.gresource.xml',
	c_name: meson.project_name()
)



configure_file(
  input: 'com.github.wwmm.easyeffects.service.in',
  output: '@0@.service'.format(app_id),
  configuration: data_conf,
  install: true,
  install_dir: join_paths(get_option('datadir'), 'dbus-1', 'services'),
)
