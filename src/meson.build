xml = configure_file(
    input: 'array.inputMethod.xml.in',
    output: 'array.xml',
    configuration: config_data
)

install_data(
    xml,
    install_dir: get_option('datadir') / 'ibus/component'
)