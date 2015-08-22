# Based on:
# https://code.google.com/p/chromium/codesearch#chromium/src/third_party/libwebp/libwebp.gyp

# Original copyright:
# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
    'variables': {'src': '<(DEPTH)/externals/libwebp/src'},
    'targets': [
        {
            'target_name': 'libwebp_dec',
            'type': 'static_library',
            'dependencies': [
                'libwebp_dsp',
                'libwebp_dsp_neon',
                'libwebp_utils',
            ],
            'include_dirs': ['<(src)'],
            'sources': [
                '<(src)/dec/alpha.c',
                '<(src)/dec/buffer.c',
                '<(src)/dec/frame.c',
                '<(src)/dec/idec.c',
                '<(src)/dec/io.c',
                '<(src)/dec/quant.c',
                '<(src)/dec/tree.c',
                '<(src)/dec/vp8.c',
                '<(src)/dec/vp8l.c',
                '<(src)/dec/webp.c',
            ],
        },
        {
            'target_name': 'libwebp_demux',
            'type': 'static_library',
            'include_dirs': ['<(src)'],
            'sources': [
                '<(src)/demux/demux.c',
            ],
        },
        {
            'target_name': 'libwebp_dsp',
            'type': 'static_library',
            'include_dirs': ['<(src)'],
            'sources': [
                '<(src)/dsp/alpha_processing.c',
                '<(src)/dsp/alpha_processing_sse2.c',
                '<(src)/dsp/cpu.c',
                '<(src)/dsp/dec.c',
                '<(src)/dsp/dec_clip_tables.c',
                '<(src)/dsp/dec_mips32.c',
                '<(src)/dsp/dec_sse2.c',
                '<(src)/dsp/enc.c',
                '<(src)/dsp/enc_avx2.c',
                '<(src)/dsp/enc_mips32.c',
                '<(src)/dsp/enc_sse2.c',
                '<(src)/dsp/lossless.c',
                '<(src)/dsp/lossless_sse2.c',
                '<(src)/dsp/upsampling.c',
                '<(src)/dsp/upsampling_sse2.c',
                '<(src)/dsp/yuv.c',
                '<(src)/dsp/yuv_mips32.c',
                '<(src)/dsp/yuv_sse2.c',
            ],
        },
        {
            'target_name': 'libwebp_dsp_neon',
            # Disable LTO due to Neon issues.
            # crbug.com/408997
            'conditions': [
                # iOS uses the same project to generate build project for both device
                # and simulator and do not use "target_arch" variable. Other platform
                # set it correctly.
                ['OS == "ios" or (target_arch == "arm" and arm_version >= 7 and (arm_neon == 1 or arm_neon_optional == 1)) or (target_arch == "arm64")',
                 {
                     'type': 'static_library',
                     'include_dirs': ['<(src)'],
                     'sources': [
                         '<(src)/dsp/dec_neon.c',
                         '<(src)/dsp/enc_neon.c',
                         '<(src)/dsp/lossless_neon.c',
                         '<(src)/dsp/upsampling_neon.c',
                     ],
                     'conditions': [
                         ['target_arch == "arm" and arm_version >= 7 and (arm_neon == 1 or arm_neon_optional == 1)',
                          {
                              # behavior similar to *.c.neon in an Android.mk
                              'cflags!': ['-mfpu=vfpv3-d16'],
                              'cflags': ['-mfpu=neon'],
                          }],
                         ['target_arch == "arm64"',
                          {
                              # avoid an ICE with gcc-4.9: b/15574841
                              'cflags': ['-frename-registers'],
                          }],
                     ]
                 },
                 {
                     'type': 'none',
                 }],
            ],
        },
        {
            'target_name': 'libwebp_enc',
            'type': 'static_library',
            'include_dirs': ['<(src)'],
            'sources': [
                '<(src)/enc/alpha.c',
                '<(src)/enc/analysis.c',
                '<(src)/enc/backward_references.c',
                '<(src)/enc/config.c',
                '<(src)/enc/cost.c',
                '<(src)/enc/filter.c',
                '<(src)/enc/frame.c',
                '<(src)/enc/histogram.c',
                '<(src)/enc/iterator.c',
                '<(src)/enc/picture.c',
                '<(src)/enc/picture_csp.c',
                '<(src)/enc/picture_psnr.c',
                '<(src)/enc/picture_rescale.c',
                '<(src)/enc/picture_tools.c',
                '<(src)/enc/quant.c',
                '<(src)/enc/syntax.c',
                '<(src)/enc/token.c',
                '<(src)/enc/tree.c',
                '<(src)/enc/vp8l.c',
                '<(src)/enc/webpenc.c',
            ],
        },
        {
            'target_name': 'libwebp_utils',
            'type': 'static_library',
            'include_dirs': ['<(src)'],
            'sources': [
                '<(src)/utils/bit_reader.c',
                '<(src)/utils/bit_writer.c',
                '<(src)/utils/color_cache.c',
                '<(src)/utils/filters.c',
                '<(src)/utils/huffman.c',
                '<(src)/utils/huffman_encode.c',
                '<(src)/utils/quant_levels.c',
                '<(src)/utils/quant_levels_dec.c',
                '<(src)/utils/random.c',
                '<(src)/utils/rescaler.c',
                '<(src)/utils/thread.c',
                '<(src)/utils/utils.c',
            ],
        },
        {
            'target_name': 'libwebp',
            'type': 'none',
            'dependencies': [
                'libwebp_dec',
                'libwebp_demux',
                'libwebp_dsp',
                'libwebp_dsp_neon',
                'libwebp_enc',
                'libwebp_utils',
            ],
            'direct_dependent_settings': {
                'include_dirs': ['<(src)'],
            },
            'conditions': [
                ['OS!="win"', {'product_name': 'webp'}],
            ],
        },
    ],
}
