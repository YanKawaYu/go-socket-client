#!/usr/bin/env python
import os
import sys
import glob

from build_utils import *

SCRIPT_PATH = os.path.split(os.path.realpath(__file__))[0]

BUILD_OUT_PATH = 'cmake_build/OSX'
INSTALL_PATH = BUILD_OUT_PATH + '/Darwin.out'

OSX_BUILD_OS_CMD = 'cmake ../.. -DSSL=%s -DLIB_NAME=%s -DCMAKE_BUILD_TYPE=Release -DENABLE_ARC=0 -DENABLE_BITCODE=0 && make -j8 && make install'

GEN_OSX_PROJ = 'cmake ../.. -G Xcode -DCMAKE_OSX_DEPLOYMENT_TARGET:STRING=10.9 -DENABLE_BITCODE=0'

def build_osx(tag=''):
    gen_revision_file('.')

    clean(BUILD_OUT_PATH)
    os.chdir(BUILD_OUT_PATH)

    ret = os.system(OSX_BUILD_OS_CMD %(WITH_SSL, LIB_NAME))
    os.chdir(SCRIPT_PATH)
    if ret != 0:
        print('!!!!!!!!!!!build fail!!!!!!!!!!!!!!!')
        return False

    libtool_os_dst_lib = INSTALL_PATH + '/' + LIB_NAME
    if not libtool_libs(glob.glob(INSTALL_PATH + '/*.a'), libtool_os_dst_lib):
        return False

    dst_framework_path = INSTALL_PATH + '/' + LIB_NAME +'.framework'
    make_static_framework(libtool_os_dst_lib, dst_framework_path, COMM_COPY_HEADER_FILES, '')

    print('==================Output========================')
    print(dst_framework_path)
    return True

def main():
    build_osx()

if __name__ == '__main__':
    main()
