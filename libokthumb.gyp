# -*- mode: python; tab-width: 2; python-indent: 2; indent-tabs-mode: nil; -*-
# https://code.google.com/p/gyp/wiki/InputFormatReference
{
    'includes': ['gyp/common.gypi'],
    'variables': {'libokthumb_maint_mode%': 0},
    'targets': [{
        'target_name': 'libokthumb',
        'dependencies': [
            'gyp/libjpeg-turbo.gyp:libjpeg-turbo', 'gyp/giflib.gyp:giflib',
            'externals/libyuv/libyuv.gyp:libyuv',
            'gyp/libwebp.gyp:libwebp'
         ],
        'conditions': [
            ['libokthumb_maint_mode == 1', {
                'cflags': [
                    '-O0', '-fno-omit-frame-pointer', '-Weverything',
                    '-Wno-c++98-compat', '-Wno-padded',
                    '-Wno-c++98-compat-pedantic', '-g',
                    '--system-header-prefix=externals'
                ],
            }],
            ['libokthumb_maint_mode == 0', {
                'cflags': ['-O3', '-fomit-frame-pointer', '-march=native',
                           '-mtune=native']
            }]
        ],
        'type': 'static_library',
        'sources': [
            'src/convert.cc',
            'src/gif_codec.cc',
            'src/image.cc',
            'src/image_reader.cc',
            'src/jpeg_codec.cc',
            'src/logging.cc',
            'src/okthumb.cc',
            'src/png_codec.cc',
            'src/ppm_codec.cc',
            'src/resize.cc',
            'src/webp_codec.cc',
        ],
        'include_dirs': [
            '<(DEPTH)'
        ],
        "cflags": [
            "-std=c++11"
        ],
        'link_settings': {
            'libraries': ['-lpng']
        },
        'direct_dependent_settings': {
            'include_dirs': [
                '<(DEPTH)', '<(DEPTH)/config'
            ],
            "cflags": [
                "-std=c++11"
            ]
        }
    }, {
        'target_name': 'decode',
        'dependencies': ['libokthumb'],
        'type': 'executable',
        'sources': ['bin/decode.cc'],
        "cflags": [
            "-std=c++11", "-lpng"
        ]
    },
    {
        'target_name': 'test_res',
        'dependencies': ['libokthumb'],
        'type': 'executable',
        'sources': ['bin/test_res.cc'],
        "cflags": [
            "-std=c++11", "-lpng"
        ]
    }]
}
