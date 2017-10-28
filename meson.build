project('PulseEffects',
  version: '3.0.6',
  meson_version: '>= 0.40.0'
)

py3_mod = import('python3')
i18n_mod = import('i18n')

message('Checking dependencies')
py3 = py3_mod.find_python()

libpulse = dependency('libpulse')
gtk3 = dependency('gtk+-3.0', version: '>= 3.18')
pygobj = dependency('pygobject-3.0')
pycairo = dependency('py3cairo')
pangocairo = dependency('pangocairo', version: '>= 1.0')

gst = dependency('gstreamer-1.0', version: '>= 1.12')
gstbad = dependency('gstreamer-plugins-bad-1.0', version: '>= 1.12')

prefix = get_option('prefix')
bindir = join_paths(prefix, get_option('bindir'))
datadir = join_paths(prefix, get_option('datadir'))
localedir = join_paths(prefix, get_option('localedir'))

# Support Debian non-standard python paths
# Fallback to Meson python3 module if command fails
message('Getting python install path')
py3_purelib = ''
r = run_command(py3.path(), '-c', 'from distutils.sysconfig import get_python_lib; print(get_python_lib(prefix=""))')
if r.returncode() != 0
  py3_purelib = py3_mod.sysconfig_path('purelib')
  if not py3_purelib.endswith('site-packages')
    error('Cannot find python install path')
  endif
  py3_dir = py3_purelib
else
  py3_dir = r.stdout().strip()
endif

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
