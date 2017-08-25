project('PulseEffects',
  version: '2.3.4'
)

py3_mod = import('python3')
i18n_mod = import('i18n')

message('Checking dependencies')
py3 = py3_mod.find_python()

message('Getting python install path')
py3_dir = py3_mod.sysconfig_path('purelib')

message('Installing PulseEffects')
install_subdir('PulseEffects', install_dir: py3_dir)
install_subdir('PulseEffectsCalibration', install_dir: py3_dir)
install_data('pulseeffects', install_dir: 'bin')

message('Installing data files')
subdir('data')
subdir('po')

meson.add_install_script('meson_post_install.py')
