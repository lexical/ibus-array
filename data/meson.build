array_db = custom_target('array.db',
    input: [
        'array30-OpenVanilla-big.cin',
        'array30-phrase-20260528.txt',
        'array-shortcode-20210725.cin',
    ],
    output: 'array.db',
    command: [
        python3, files('gen_array_db.py'),
        '@INPUT0@', '@INPUT1@', '@INPUT2@', '@OUTPUT@',
    ],
    install: true,
    install_dir: get_option('datadir') / meson.project_name() / 'tables'
)
