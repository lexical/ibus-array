project(
    'ibus-array',
    'c',
    license: 'GPL-2.0-or-later',
    version: '0.2.2'
)

python = import('python')

python3 = python.find_installation('python3')
ibus = dependency('ibus-1.0')
sqlite3 = dependency('sqlite3')
opencc = dependency('opencc', required: get_option('enable_opencc'))

config_data = configuration_data()
config_data.set('PACKAGE', meson.project_name())
config_data.set('VERSION', meson.project_version())
config_data.set('prefix', get_option('prefix'))
config_data.set('PYTHON', python3.path())
config_data.set('LOCALEDIR', get_option('prefix') / get_option('localedir'))
config_data.set('LIBEXECDIR', get_option('prefix') / get_option('libexecdir'))
config_data.set('PKGDATADIR', get_option('prefix') / get_option('datadir') / meson.project_name())
if get_option('enable_opencc').enabled()
    config_data.set10('HAVE_OPENCC', true)
endif
configure_file(
    input: 'config.h.in',
    output: 'config.h',
    configuration: config_data
)
subdir('data')
subdir('icons')
subdir('po')
subdir('src')
subdir('setup')

sources = files([
    'src/array.c',
    'src/engine.c',
    'src/main.c'
])

dependencies = [ibus, sqlite3]
if get_option('enable_opencc').enabled()
    dependencies += opencc
endif

executable(
    'ibus-engine-array',
    sources,
    dependencies: dependencies,
    install: true,
    install_dir: get_option('libexecdir')
)