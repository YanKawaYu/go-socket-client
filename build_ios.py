#!/usr/bin/env python
import os
import sys
import glob

from build_utils import *

SCRIPT_PATH = os.path.split(os.path.realpath(__file__))[0]

BUILD_OUT_PATH = 'cmake_build/iOS'
INSTALL_PATH = BUILD_OUT_PATH + '/Darwin.out'

IOS_BUILD_SIMULATOR_CMD = 'cmake ../.. -DSSL=%s -DLIB_NAME=%s -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=../../ios.toolchain.cmake -DIOS_PLATFORM=SIMULATOR -DIOS_ARCH="x86_64" -DENABLE_ARC=0 -DENABLE_BITCODE=1 -DENABLE_VISIBILITY=1 && make -j8 && make install'
IOS_BUILD_OS_CMD = 'cmake ../.. -DSSL=%s -DLIB_NAME=%s -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=../../ios.toolchain.cmake -DIOS_PLATFORM=OS -DIOS_ARCH="armv7;arm64" -DENABLE_ARC=0 -DENABLE_BITCODE=1 -DENABLE_VISIBILITY=1 && make -j8 && make install'

def build_ios():
    gen_revision_file('.')

    clean(BUILD_OUT_PATH)
    os.chdir(BUILD_OUT_PATH)

    ret = os.system(IOS_BUILD_OS_CMD % (WITH_SSL, LIB_NAME))
    os.chdir(SCRIPT_PATH)
    if ret != 0:
        print('!!!!!!!!!!!build os fail!!!!!!!!!!!!!!!')
        return False

    libtool_os_dst_lib = INSTALL_PATH + '/os'
    if not libtool_libs(glob.glob(INSTALL_PATH + '/*.a'), libtool_os_dst_lib):
        return False

    clean(BUILD_OUT_PATH)
    os.chdir(BUILD_OUT_PATH)
    ret = os.system(IOS_BUILD_SIMULATOR_CMD % (WITH_SSL, LIB_NAME))
    os.chdir(SCRIPT_PATH)
    if ret != 0:
        print('!!!!!!!!!!!build simulator fail!!!!!!!!!!!!!!!')
        return False

    libtool_simulator_dst_lib = INSTALL_PATH + '/simulator'
    if not libtool_libs(glob.glob(INSTALL_PATH + '/*.a'), libtool_simulator_dst_lib):
        return False

    lipo_src_libs = []
    lipo_src_libs.append(libtool_os_dst_lib)
    lipo_src_libs.append(libtool_simulator_dst_lib)
    lipo_dst_lib = INSTALL_PATH + '/noopenssl'

    if not lipo_libs(lipo_src_libs, lipo_dst_lib):
        return False

    openssl_src_libs = []
    openssl_src_libs.append(lipo_dst_lib)

    if WITH_SSL == "1":
        openssl_src_libs.append('openssl/ios/libcrypto.a')
        openssl_src_libs.append('openssl/ios/libssl.a')

    openssl_dst_lib = INSTALL_PATH + '/' + LIB_NAME
    if not libtool_libs(openssl_src_libs, openssl_dst_lib):
        return False

    dst_framework_path = INSTALL_PATH + '/' + LIB_NAME + '.framework'
    make_static_framework(openssl_dst_lib, dst_framework_path, COMM_COPY_HEADER_FILES, '')

    print('==================Output========================')
    print(dst_framework_path)
    return True

def main():
    build_ios()

if __name__ == '__main__':
    main()
