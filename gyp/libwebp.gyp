# Taken from https://github.com/google/skia/blob/master/gyp/libwebp.gyp
# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
    'variables': {'SRC': '<(DEPTH)/externals/libwebp'},
    'targets': [
        {
            'target_name': 'libwebp_dec',
            'type': 'static_library',
            'include_dirs': [
                '<(SRC)',
            ],
            'sources': [
                '<(SRC)/src/dec/alpha.c',
                '<(SRC)/src/dec/buffer.c',
                '<(SRC)/src/dec/frame.c',
                '<(SRC)/src/dec/idec.c',
                '<(SRC)/src/dec/io.c',
                '<(SRC)/src/dec/quant.c',
                '<(SRC)/src/dec/tree.c',
                '<(SRC)/src/dec/vp8.c',
                '<(SRC)/src/dec/vp8l.c',
                '<(SRC)/src/dec/webp.c',
            ],
        },
        {
            'target_name': 'libwebp_demux',
            'type': 'static_library',
            'include_dirs': [
                '<(SRC)',
            ],
            'sources': [
                '<(SRC)/src/demux/demux.c',
            ],
        },
        {
            'target_name': 'libwebp_dsp',
            'type': 'static_library',
            'include_dirs': [
                '<(SRC)',
            ],
            'sources': [
                '<(SRC)/src/dsp/alpha_processing.c',
                '<(SRC)/src/dsp/alpha_processing_mips_dsp_r2.c',
                '<(SRC)/src/dsp/alpha_processing_sse2.c',
                '<(SRC)/src/dsp/alpha_processing_sse41.c',
                '<(SRC)/src/dsp/cpu.c',
                '<(SRC)/src/dsp/dec.c',
                '<(SRC)/src/dsp/dec_clip_tables.c',
                '<(SRC)/src/dsp/dec_mips32.c',
                '<(SRC)/src/dsp/dec_mips_dsp_r2.c',
                '<(SRC)/src/dsp/dec_sse2.c',
                '<(SRC)/src/dsp/dec_sse41.c',
                '<(SRC)/src/dsp/enc.c',
                '<(SRC)/src/dsp/enc_sse2.c',
                '<(SRC)/src/dsp/filters.c',
                '<(SRC)/src/dsp/filters_mips_dsp_r2.c',
                '<(SRC)/src/dsp/filters_sse2.c',
                '<(SRC)/src/dsp/lossless.c',
                '<(SRC)/src/dsp/lossless_mips_dsp_r2.c',
                '<(SRC)/src/dsp/lossless_sse2.c',
                '<(SRC)/src/dsp/rescaler.c',
                '<(SRC)/src/dsp/rescaler_mips32.c',
                '<(SRC)/src/dsp/rescaler_mips_dsp_r2.c',
                '<(SRC)/src/dsp/upsampling.c',
                '<(SRC)/src/dsp/upsampling_mips_dsp_r2.c',
                '<(SRC)/src/dsp/upsampling_sse2.c',
                '<(SRC)/src/dsp/yuv.c',
                '<(SRC)/src/dsp/yuv_mips32.c',
                '<(SRC)/src/dsp/yuv_mips_dsp_r2.c',
                '<(SRC)/src/dsp/yuv_sse2.c',
            ],
        },
        {
            'target_name': 'libwebp_dsp_neon',
            'conditions': [
                # iOS uses the same project to generate build project for both device
                # and simulator and do not use "target_arch" variable. Other platform
                # set it correctly.
                ['OS == "ios" or (target_arch == "arm" and arm_version >= 7 and (arm_neon == 1 or arm_neon_optional == 1)) or (target_arch == "arm64")',
                 {
                     'type': 'static_library',
                     'include_dirs': ['<(SRC)/src'],
                     'sources': [
                         '<(SRC)/src/dsp/dec_neon.c',
                         '<(SRC)/src/dsp/enc_neon.c',
                         '<(SRC)/src/dsp/lossless_neon.c',
                         '<(SRC)/src/dsp/upsampling_neon.c',
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
            ]
        },
        {
            'target_name': 'libwebp_dsp_enc',
            'type': 'static_library',
            'include_dirs': [
                '<(SRC)',
            ],
            'sources': [
                '<(SRC)/src/dsp/argb.c',
                '<(SRC)/src/dsp/argb_mips_dsp_r2.c',
                '<(SRC)/src/dsp/argb_sse2.c',
                '<(SRC)/src/dsp/cost.c',
                '<(SRC)/src/dsp/cost_mips32.c',
                '<(SRC)/src/dsp/cost_mips_dsp_r2.c',
                '<(SRC)/src/dsp/cost_sse2.c',
                '<(SRC)/src/dsp/enc_avx2.c',
                '<(SRC)/src/dsp/enc_mips32.c',
                '<(SRC)/src/dsp/enc_mips_dsp_r2.c',
                '<(SRC)/src/dsp/enc_sse41.c',
                '<(SRC)/src/dsp/lossless_enc.c',
                '<(SRC)/src/dsp/lossless_enc_mips32.c',
                '<(SRC)/src/dsp/lossless_enc_mips_dsp_r2.c',
                '<(SRC)/src/dsp/lossless_enc_sse2.c',
                '<(SRC)/src/dsp/lossless_enc_sse41.c',
            ],
        },
        {
            'target_name': 'libwebp_enc',
            'type': 'static_library',
            'include_dirs': [
                '<(SRC)',
            ],
            'dependencies': [
                'libwebp_dsp_enc',
            ],
            'sources': [
                '<(SRC)/src/enc/alpha.c',
                '<(SRC)/src/enc/analysis.c',
                '<(SRC)/src/enc/backward_references.c',
                '<(SRC)/src/enc/config.c',
                '<(SRC)/src/enc/cost.c',
                '<(SRC)/src/enc/filter.c',
                '<(SRC)/src/enc/frame.c',
                '<(SRC)/src/enc/histogram.c',
                '<(SRC)/src/enc/iterator.c',
                '<(SRC)/src/enc/near_lossless.c',
                '<(SRC)/src/enc/picture.c',
                '<(SRC)/src/enc/picture_csp.c',
                '<(SRC)/src/enc/picture_psnr.c',
                '<(SRC)/src/enc/picture_rescale.c',
                '<(SRC)/src/enc/picture_tools.c',
                '<(SRC)/src/enc/quant.c',
                '<(SRC)/src/enc/syntax.c',
                '<(SRC)/src/enc/token.c',
                '<(SRC)/src/enc/tree.c',
                '<(SRC)/src/enc/vp8l.c',
                '<(SRC)/src/enc/webpenc.c',
            ],
        },
        {
            'target_name': 'libwebp_utils',
            'type': 'static_library',
            'include_dirs': [
                '<(SRC)',
            ],
            'sources': [
                '<(SRC)/src/utils/bit_reader.c',
                '<(SRC)/src/utils/bit_writer.c',
                '<(SRC)/src/utils/color_cache.c',
                '<(SRC)/src/utils/filters.c',
                '<(SRC)/src/utils/huffman.c',
                '<(SRC)/src/utils/huffman_encode.c',
                '<(SRC)/src/utils/quant_levels.c',
                '<(SRC)/src/utils/quant_levels_dec.c',
                '<(SRC)/src/utils/random.c',
                '<(SRC)/src/utils/rescaler.c',
                '<(SRC)/src/utils/thread.c',
                '<(SRC)/src/utils/utils.c',
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
                'include_dirs': [
                    '<(SRC)/src',
                ],
            },
            'conditions': [
                ['OS!="win"', {'product_name': 'webp'}],
            ],
        },
    ],
}
