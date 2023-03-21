project(
	'easyeffects',
	'c',
	'cpp',
	default_options : [
    'cpp_std=c++20',
    'buildtype=debugoptimized',
    # 1: -Wall
    # 2: -Wall -Wextra
    # 3: -Wall -Wextra -Wpedantic
    'warning_level=2'
  ],
  license: 'GPL-3-or-later',
	version: '7.0.1',
	meson_version: '>= 0.59.0'
)

suppressed_warnings = [
  '-Wno-missing-field-initializers',
  '-Wno-unused-parameter'
  ]

add_project_arguments (
  suppressed_warnings,
  language: [ 'c', 'cpp' ],
  )

add_global_arguments('-DG_LOG_DOMAIN="easyeffects"', language : [ 'c', 'cpp' ])
add_global_arguments('-DGETTEXT_PACKAGE="@0@"'.format(meson.project_name()),
	language:'c')

gnome_mod = import('gnome')
i18n_mod = import('i18n')

prefix = get_option('prefix')
bindir = join_paths(prefix, get_option('bindir'))
datadir = join_paths(prefix, get_option('datadir'))
localedir = join_paths(prefix, get_option('localedir'))
include_dir = include_directories('include')
config_h_dir = include_directories('.')

libportal = dependency('libportal-gtk4', include_type: 'system', required: false)

status = []

if get_option('devel')
  status += [ 
    'Using development build mode with .Devel appended to the application ID.',
    'Also using mold linker, so make sure mold is installed on your system.'
  ]
  app_id_suffix = '.Devel'
  name_suffix = ' (Devel)'
  # --no-as-needed temporarily necessary due to https://github.com/rui314/mold/issues/1017
  link_args = ['-fuse-ld=mold', '-Wl,--no-as-needed']
else
  status += [ 'Using stable build mode with the standard application ID' ]
  app_id_suffix = ''
  name_suffix = ''
  link_args = []
endif

if get_option('enable-libportal') and libportal.found()
  add_global_arguments('-DUSE_LIBPORTAL=1', language : 'cpp')

  message('Using libportal to handle autostart files')
endif

app_id = 'com.github.wwmm.easyeffects@0@'.format(app_id_suffix)

# Configuration for application itself at compile time, such as debug info like the commit that was built
conf = configuration_data()

conf.set10('IS_DEVEL_BUILD', get_option('devel'))
conf.set_quoted('APP_NAME', 'Easy Effects' + name_suffix)

conf.set_quoted('GETTEXT_PACKAGE', meson.project_name())
conf.set_quoted('LOCALE_DIR', localedir)
conf.set_quoted('VERSION', meson.project_version())

conf.set('COMMIT_DESC', '"@VCS_TAG@"')

config_h = declare_dependency(
  sources: vcs_tag(
    command: ['git', 'rev-parse', '--short', 'HEAD'],
    fallback: 'Could not find commit of build',
    input: configure_file(
      output: 'config.h.in',
      configuration: conf
    ),
    output: 'config.h'
  )
)

subdir('data')
subdir('po')
subdir('help')
subdir('src')

gnome_mod.post_install(
  glib_compile_schemas: true,
  gtk_update_icon_cache: true,
  update_desktop_database: true,
)

message('\n  '.join(status))
