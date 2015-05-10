{
  'variables': { 'src': '<(DEPTH)/externals/giflib/lib' },
  'targets': [
    {
      'target_name': 'giflib',
      'type': 'static_library',
      'include_dirs': [ '<(src)/' ],
      'dependencies': [
      ],
      'cflags': [
        '-w',
      ],
      'sources': [
        '<(src)/dgif_lib.c',
        '<(src)/egif_lib.c',
        '<(src)/gif_font.c',
        '<(src)/gif_hash.c',
        '<(src)/gif_hash.h',
        '<(src)/gifalloc.c',
        '<(src)/gif_err.c',
        '<(src)/gif_lib_private.h',
        '<(src)/quantize.c'
      ],
      'direct_dependent_settings': {
          'include_dirs': [
              '<(src)',
          ],
       }
    }
  ]
}
