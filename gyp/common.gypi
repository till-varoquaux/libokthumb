{
    'variables': {
        'variables': { 'host_arch%': '<!(python <(DEPTH)/gyp/detect_host_arch.py)' },
        'target_arch%': '<(host_arch)',
        'msan%': 0,
        'libyuv_disable_jpeg%': 1
    }
}
