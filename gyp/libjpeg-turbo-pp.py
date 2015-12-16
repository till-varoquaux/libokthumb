#!/usr/bin/env python

import argparse
import subprocess
import os
import os.path


def process(args):
    r"""
    Run cpp and substitute the output like the following automake command

    $(AM_V_GEN) $(CPP) -I$(top_builddir) \
     -I$(top_builddir)/simd $(srcdir)/jsimdcfg.inc.h
     | $(EGREP) "^[\;%]|^\ %"
     | sed 's%_cpp_protection_%%'
     | sed 's@% define@%define@g' > $@
    """
    cmd = ["cpp"]
    for i in args.includes:
        cmd += ["-I", i]
    cmd += [args.in_file]
    out = subprocess.check_output(cmd).split('\n')
    dirname = os.path.dirname(args.out_file)
    if not os.path.exists(dirname):
        os.mkdir(dirname)
    with open(args.out_file, 'w') as tgt:
        for line in out:
            if line.startswith('%') or line.startswith(';') \
              or line.startswith(' %'):
                line = line.replace('_cpp_protection_', '')\
                  .replace('% define', '%define')
                tgt.write(line + '\n')


def main():
    r"""
    Entry point for the programm
    """
    parser = argparse.ArgumentParser('Preprocess libjpeg-turbo files.')
    parser.add_argument('-I',
                        dest='includes',
                        nargs=1,
                        action='append',
                        help='An include to pass on to CPP')
    parser.add_argument(dest='in_file', nargs=1, type=str, help='in file')
    parser.add_argument(dest='out_file', nargs=1, type=str, help='out file')
    args = parser.parse_args()
    args.in_file = args.in_file[0]
    args.out_file = args.out_file[0]
    if args.includes is None:
        args.includes = []
    else:
        args.includes = [x[0] for x in args.includes]
    process(args)

if __name__ == '__main__':
    main()
