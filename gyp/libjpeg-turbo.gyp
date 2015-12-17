# -*- mode: python; tab-width: 2; python-indent: 2; indent-tabs-mode: nil; -*-
# Based on:
# https://code.google.com/p/chromium/codesearch#chromium/src/third_party/libjpeg_turbo/libjpeg.gyp
# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
    'includes': ['common.gypi'],
    'variables': {
        'shared_generated_dir':
        '<(SHARED_INTERMEDIATE_DIR)/externals/libjpeg-turbo',
        'src': '<(DEPTH)/externals/libjpeg-turbo'
    },
    'targets': [
        {
        'target_name': 'jpegtran',
        'dependencies': ['libokthumb'],
        'type': 'executable',
        'sources': ['<(src)/jpegtran.c'],
        'dependencies': ["libjpeg-turbo"],
        "cflags": [
        ]},
        {
            'target_name': 'libjpeg-turbo',
            'type': 'static_library',
            'include_dirs': [
                '<(shared_generated_dir)', '<(DEPTH)/config'
            ],
            'direct_dependent_settings': {
                'include_dirs': [
                    '<(DEPTH)/config',
                    '<(src)',
                ],
            },
            'defines': [
                'NO_GETENV',
            ],
            'sources': [
                '<(src)/jaricom.c',
                '<(src)/jcapimin.c',
                '<(src)/jcapistd.c',
                '<(src)/jccoefct.c',
                '<(src)/jccolor.c',
                '<(src)/jcdctmgr.c',
                '<(src)/jchuff.c',
                '<(src)/jchuff.h',
                '<(src)/jcinit.c',
                '<(src)/jcarith.c',
                '<(src)/jctrans.c',
                '<(src)/jcmainct.c',
                '<(src)/jcmarker.c',
                '<(src)/jcmaster.c',
                '<(src)/jcomapi.c',
                '<(DEPTH)/config/jconfig.h',
                '<(DEPTH)/config/jconfigint.h',
                '<(src)/jcparam.c',
                '<(src)/jcphuff.c',
                '<(src)/jcprepct.c',
                '<(src)/jcsample.c',
                '<(src)/jdapimin.c',
                '<(src)/jdapistd.c',
                '<(src)/jdarith.c',
                '<(src)/jdatadst.c',
                '<(src)/jdatasrc.c',
                '<(src)/jdcoefct.c',
                '<(src)/jdcolor.c',
                '<(src)/jdct.h',
                '<(src)/jddctmgr.c',
                '<(src)/jdhuff.c',
                '<(src)/jdhuff.h',
                '<(src)/jdinput.c',
                '<(src)/jdmainct.c',
                '<(src)/jdmarker.c',
                '<(src)/jdmaster.c',
                '<(src)/jdmerge.c',
                '<(src)/jdphuff.c',
                '<(src)/jdpostct.c',
                '<(src)/jdsample.c',
                '<(src)/jdtrans.c',
                '<(src)/jerror.c',
                '<(src)/jerror.h',
                '<(src)/jfdctflt.c',
                '<(src)/jfdctfst.c',
                '<(src)/jfdctint.c',
                '<(src)/jidctflt.c',
                '<(src)/jidctfst.c',
                '<(src)/jidctint.c',
                '<(src)/jidctred.c',
                '<(src)/jinclude.h',
                '<(src)/jmemmgr.c',
                '<(src)/jmemnobs.c',
                '<(src)/jmemsys.h',
                '<(src)/jmorecfg.h',
                '<(src)/jpegint.h',
                '<(src)/jpeglib.h',
                '<(src)/jpeglibmangler.h',
                '<(src)/jquant1.c',
                '<(src)/jquant2.c',
                '<(src)/jutils.c',
                '<(src)/jversion.h',
                '<(src)/transupp.c',
                '<(src)/cdjpeg.c',
                '<(src)/rdswitch.c'
            ],
            'conditions': [
                # Add target-specific source files.
                ['target_arch=="x64" and msan!=1', {
                    'sources': [
                        '<(shared_generated_dir)/simd/jsimdcfg.inc',
                        '<(src)/simd/jsimd_x86_64.c', '<(src)/simd/jsimd.h',
                        '<(src)/simd/jsimdext.inc', '<(src)/simd/jcolsamp.inc',
                        '<(src)/simd/jdct.inc',
                        '<(src)/simd/jfdctflt-sse-64.asm',
                        '<(src)/simd/jccolor-sse2-64.asm',
                        '<(src)/simd/jcgray-sse2-64.asm',
                        '<(src)/simd/jcsample-sse2-64.asm',
                        '<(src)/simd/jdcolor-sse2-64.asm',
                        '<(src)/simd/jdmerge-sse2-64.asm',
                        '<(src)/simd/jdsample-sse2-64.asm',
                        '<(src)/simd/jfdctfst-sse2-64.asm',
                        '<(src)/simd/jfdctint-sse2-64.asm',
                        '<(src)/simd/jidctflt-sse2-64.asm',
                        '<(src)/simd/jidctfst-sse2-64.asm',
                        '<(src)/simd/jidctint-sse2-64.asm',
                        '<(src)/simd/jidctred-sse2-64.asm',
                        '<(src)/simd/jquantf-sse2-64.asm',
                        '<(src)/simd/jquanti-sse2-64.asm'
                    ],
                }],
                # MemorySanitizer doesn't support assembly code, so keep it disabled in
                # MSan builds for now.
                ['msan==1', {
                    'sources': [
                        '<(src)/jsimd_none.c',
                    ],
                }],
                ['OS=="linux"',
                 {
                     'variables': {
                         'yasm_format': '-felf64',
                         'yasm_flags': [
                             '-D__x86_64__', '-DELF', '-Ilinux/',
                             '-I<(shared_generated_dir)/simd'
                         ],
                     },
                 }, ],
            ],
            'actions': [
                {
                    'action_name': 'gen_simd_inc',
                    'inputs': [
                        '<(src)/simd/jsimdcfg.inc.h',
                        '<(DEPTH)/config/jconfig.h'
                    ],
                    'outputs': [
                        '<(shared_generated_dir)/simd/jsimdcfg.inc',
                    ],
                    'action': ['python',
                               'libjpeg-turbo-pp.py',
                               '-I<(src)',
                               '-I<(src)/simd',
                               '-I<(shared_generated_dir)',
                               '-I<(DEPTH)/config',
                               '-I<(DEPTH)/config/simd',  #mkdir here...
                               '<(src)/simd/jsimdcfg.inc.h',
                               '<@(_outputs)'],
                    'message': 'generate inc file',
                }
            ],
            'rules': [
                {
                    'rule_name': 'assemble',
                    'extension': 'asm',
                    'conditions': [
                        ['target_arch=="ia32" or target_arch=="x64"', {
                            'inputs': [],
                            'outputs': [
                                '<(shared_generated_dir)/<(RULE_INPUT_ROOT).o',
                            ],
                            'action': [
                                'yasm',
                                '<(yasm_format)',
                                '<@(yasm_flags)',
                                '-DRGBX_FILLER_0XFF',
                                '-DSTRICT_MEMORY_ACCESS',
                                '-Isimd/',
                                '-I<(shared_generated_dir)/',
                                '-o',
                                '<(shared_generated_dir)/<(RULE_INPUT_ROOT).o',
                                '<(RULE_INPUT_PATH)',
                            ],
                            'process_outputs_as_sources': 1,
                            'message': 'Building <(RULE_INPUT_ROOT).o',
                        }],
                    ]
                },
            ],
        },
    ],
}

# Local Variables:
# tab-width:2
# indent-tabs-mode:nil
# End:
# vim: set expandtab tabstop=2 shiftwidth=2:
