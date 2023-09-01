#!/usr/bin/env python
import os
import sys
import glob
import time
import shutil
import platform

from build_utils import *

SCRIPT_PATH = os.path.split(os.path.realpath(__file__))[0]

def system_is_windows():
    return platform.system() == 'Windows'

def system_architecture_is64():
    return platform.machine().endswith('64')


if system_is_windows():
    ANDROID_GENERATOR = '-G "Unix Makefiles"'
else:
    ANDROID_GENERATOR = ''

try:
    NDK_ROOT = os.environ['NDK_ROOT']
except KeyError as identifier:
    NDK_ROOT = ''

BUILD_OUT_PATH = 'cmake_build/Android'
ANDROID_LIBS_INSTALL_PATH = BUILD_OUT_PATH + '/'
ANDROID_LOG_INSTALL_PATH = ANDROID_LIBS_INSTALL_PATH + 'log/'
ANDROID_BUILD_CMD = 'cmake "%s" %s -DSSL="%s" -DLIB_NAME="%s" -DANDROID_ABI="%s" -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=%s/build/cmake/android.toolchain.cmake -DANDROID_TOOLCHAIN=gcc -DANDROID_NDK=%s -DANDROID_PLATFORM=android-21 -DANDROID_STL="c++_shared" && cmake --build . %s --config Release -- -j8'
ANDROID_SYMBOL_PATH = 'cmake_build/Android_out/obj/local/'
ANDROID_LIBS_PATH = 'cmake_build/Android_out/libs/'

ANDROID_STRIP_FILE = {
        'armeabi': NDK_ROOT + '/toolchains/arm-linux-androideabi-4.9/prebuilt/%s/bin/arm-linux-androideabi-strip',
        'armeabi-v7a': NDK_ROOT + '/toolchains/arm-linux-androideabi-4.9/prebuilt/%s/bin/arm-linux-androideabi-strip',
        'x86': NDK_ROOT + '/toolchains/x86-4.9/prebuilt/%s/bin/i686-linux-android-strip',
        'arm64-v8a': NDK_ROOT + '/toolchains/aarch64-linux-android-4.9/prebuilt/%s/bin/aarch64-linux-android-strip',
        'x86_64': NDK_ROOT + '/toolchains/x86_64-4.9/prebuilt/%s/bin/x86_64-linux-android-strip',
         }


ANDROID_STL_FILE = {
        'armeabi': NDK_ROOT + '/sources/cxx-stl/llvm-libc++/libs/armeabi/libc++_shared.so',
        'armeabi-v7a': NDK_ROOT + '/sources/cxx-stl/llvm-libc++/libs/armeabi-v7a/libc++_shared.so',
        'x86': NDK_ROOT + '/sources/cxx-stl/llvm-libc++/libs/x86/libc++_shared.so',
        'arm64-v8a': NDK_ROOT + '/sources/cxx-stl/llvm-libc++/libs/arm64-v8a/libc++_shared.so',
        'x86_64': NDK_ROOT + '/sources/cxx-stl/llvm-libc++/libs/x86_64/libc++_shared.so',
        }

def check_ndk_env():
    try:
        ndk_path = os.environ['NDK_ROOT']
    except KeyError as identifier:
        print("Error: ndk does not exist or you do not set it into NDK_ROOT.")
        return False

    if ndk_path is not None and ndk_path.strip():
        print("ndk path:%s"%ndk_path)

    if not ndk_path:
        print("Error: ndk does not exist or you do not set it into NDK_ROOT.")
        return False

    if not os.path.isfile(os.path.join(ndk_path, "source.properties")):
        print("Error: source.properties does not exist, make sure ndk's version==r16b")
        return False

    ndk_revision = None

    f = open(os.path.join(ndk_path, "source.properties"))
    line = f.readline()
    while line:
        if line.startswith("Pkg.Revision") and len(line.split("=")) == 2:
            ndk_revision = line.split("=")[1].strip()
        line = f.readline()

    f.close()

    if not ndk_revision or len(ndk_revision) < 4:
        print("Error: parse source.properties fail")
        return False

    if ndk_revision[:4] >= "16.1" and ndk_revision[:4] < '16.2':
        return True

    print("Error: make sure ndk's version == r16b")
    return False

def get_android_strip_cmd(arch):

    system_str = platform.system().lower()
    if (system_architecture_is64()):
        system_str = system_str + '-x86_64'
    else:
        pass

    strip_cmd = ANDROID_STRIP_FILE[arch] %(system_str)
    print('Android strip cmd:%s' %(strip_cmd))
    return strip_cmd


def build_android(incremental, arch, target_option=''):
    gen_revision_file('.')

    before_time = time.time()

    clean(BUILD_OUT_PATH, incremental)
    os.chdir(BUILD_OUT_PATH)

    build_cmd = ANDROID_BUILD_CMD %(SCRIPT_PATH, ANDROID_GENERATOR, WITH_SSL, LIB_NAME, arch, NDK_ROOT, NDK_ROOT, target_option)
    print("build cmd:" + build_cmd)
    ret = os.system(build_cmd)
    os.chdir(SCRIPT_PATH)

    if 0 != ret:
        print('!!!!!!!!!!!!!!!!!!build fail!!!!!!!!!!!!!!!!!!!!')
        return False

    symbol_path = ANDROID_SYMBOL_PATH
    lib_path = ANDROID_LIBS_PATH

    if not os.path.exists(symbol_path):
        os.makedirs(symbol_path)

    symbol_path = symbol_path + arch
    if os.path.exists(symbol_path):
        shutil.rmtree(symbol_path)

    os.mkdir(symbol_path)

    if not os.path.exists(lib_path):
        os.makedirs(lib_path)

    lib_path = lib_path + arch
    if os.path.exists(lib_path):
        shutil.rmtree(lib_path)

    os.mkdir(lib_path)

    for f in glob.glob(ANDROID_LIBS_INSTALL_PATH + "*.so"):
        shutil.copy(f, symbol_path)
        shutil.copy(f, lib_path)

    # copy stl
    shutil.copy(ANDROID_STL_FILE[arch], symbol_path)
    shutil.copy(ANDROID_STL_FILE[arch], lib_path)

    #strip
    strip_cmd = get_android_strip_cmd(arch)
    for f in glob.glob('%s/*.so' %(lib_path)):
        os.system('%s %s' %(strip_cmd, f))

    print('==================Output========================')
    print('libs(release): %s' %(lib_path))
    print('symbols(must store permanently): %s' %(symbol_path))

    after_time = time.time()

    print("use time:%d s" % (int(after_time - before_time)))
    return True

def main(incremental, archs, target_option='', tag=''):
    if not check_ndk_env():
        return

    if os.path.exists(ANDROID_LIBS_PATH):
        shutil.rmtree(ANDROID_LIBS_PATH)

    if os.path.exists(ANDROID_SYMBOL_PATH):
        shutil.rmtree(ANDROID_SYMBOL_PATH)

    for arch in archs:
        if not build_android(incremental, arch, target_option):
            return

if __name__ == '__main__':
    archs = set(['armeabi-v7a'])

    # with simulator support
    # archs = set(['arm64-v8a','armeabi-v7a','x86_64'])
    main(False, archs)
