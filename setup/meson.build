config = configure_file(
    input: 'config.py.in',
    output: 'config.py',
    configuration: config_data
)
setup = configure_file(
    input: 'ibus-setup-array.in',
    output: 'ibus-setup-array',
    configuration: config_data
)

install_data(
    config,
    'main.py',
    install_dir: get_option('datadir') / meson.project_name() / 'setup'
)
install_data(
    setup,
    install_mode: 'rwxr-xr-x',
    install_dir: get_option('libexecdir')
)