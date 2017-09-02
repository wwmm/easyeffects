project('PulseEffects',
  version: '3.0.0'
)

py3_mod = import('python3')
i18n_mod = import('i18n')

message('Checking dependencies')
py3 = py3_mod.find_python()

gtk3 = dependency('gtk+-3.0', version: '>= 3.20')
pygobj = dependency('pygobject-3.0')
pycairo = dependency('py3cairo')

gst = dependency('gstreamer-1.0', version: '>= 1.12')
gstbad = dependency('gstreamer-plugins-bad-1.0', version: '>= 1.12')

prefix = get_option('prefix')
bindir = join_paths(prefix, get_option('bindir'))
datadir = join_paths(prefix, get_option('datadir'))
localedir = join_paths(prefix, get_option('localedir'))

message('Getting python install path')
py3_dir = py3_mod.sysconfig_path('purelib')

message('Installing PulseEffects')
install_subdir('PulseEffects', install_dir: py3_dir)
install_subdir('PulseEffectsCalibration', install_dir: py3_dir)

conf_pulseeffects = configuration_data()
conf_pulseeffects.set('LOCALEDIR', localedir)
conf_pulseeffects.set('PROJECT_NAME', meson.project_name())
configure_file(input: 'pulseeffects.in',
               output: 'pulseeffects',
               configuration: conf_pulseeffects,
               install_dir: bindir)

message('Installing data files')
subdir('data')
subdir('po')

meson.add_install_script('meson_post_install.py')
