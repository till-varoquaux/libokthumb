# -*- mode: python; tab-width: 2; python-indent: 2; indent-tabs-mode: nil; -*-
# https://code.google.com/p/gyp/wiki/InputFormatReference
{
    'includes': ['gyp/common.gypi'],
    'variables': { 'libokthumb_maint_mode%': 0 },
    'targets': [{
        'target_name': 'libokthumb',
        'dependencies': [
            'gyp/libjpeg-turbo.gyp:libjpeg-turbo',
            'gyp/giflib.gyp:giflib',
            'externals/libyuv/libyuv.gyp:libyuv',
            'gyp/libwebp.gyp:*'
         ],
         'conditions': [
             ['libokthumb_maint_mode == 1', {
                 'cflags': [
                     '-O0',
                     '-fno-omit-frame-pointer',
                     '-Weverything',
                     '-Wno-c++98-compat',
                     '-Wno-padded',
                     '-Wno-c++98-compat-pedantic',
                     '-g',
                     '--system-header-prefix=externals'
                    ],
                }]
            ],
            'type': 'static_library',
            'sources': [
                'src/aligned_storage.h',
                'src/bounded_int.h',
                'src/gif_codec.cc',
                'src/gif_codec.h',
                'src/image.cc',
                'src/image.h',
                'src/image_reader.cc',
                'src/image_reader.h',
                'src/indice_tuple.h',
                'src/jpeg_codec.cc',
                'src/jpeg_codec.h',
                'src/jpeg_config-x.def',
                'src/logging.cc',
                'src/logging.h',
                'src/planar_image.h',
                'src/png_codec.cc',
                'src/png_codec.h',
                'src/ppm_codec.cc',
                'src/ppm_codec.h',
                'src/resize.cc',
                'src/resize_config-x.def',
                'src/resize.h',
                'src/resize_tmpl.h',
                'src/webp_codec.cc',
                'src/webp_codec.h'
          ],
          'include_dirs': [
              'src',
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
                  'src',
              ],
              "cflags": [
                  "-std=c++11"
              ]
           }
    },
    {
        'target_name': 'decode',
        'dependencies': ['libokthumb'],
        'type': 'executable',
        'sources': [ 'bin/decode.cc' ],
        "cflags": [
            "-std=c++11", "-lpng"
        ]
    }
    ]
}
