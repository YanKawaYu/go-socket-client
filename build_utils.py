#!/usr/bin/env python
import os
import shutil
import time
import glob
import getopt
import sys

# the name of the library
LIB_NAME = "gotcp"

# whether to build the library with ssl
WITH_SSL = "1"
if len(sys.argv)>1:
    if sys.argv[1] == "no_ssl":
        WITH_SSL = "0"

COMM_COPY_HEADER_FILES = {
            "net_core.hpp": "",
            "tcp_define.h": "",
            "log/log_define.h": "",
            }

def parse_as_git(path):
    curdir = os.getcwd()
    os.chdir(path)
    revision = os.popen('git rev-parse --short HEAD').read().strip()
    path = os.popen('git rev-parse --abbrev-ref HEAD').read().strip()
    os.chdir(curdir)

    return revision, path

def gen_revision_file(version_file_path, tag=''):
    revision, path = parse_as_git(version_file_path)

    build_time = time.strftime("%Y-%m-%d %H:%M:%S", time.localtime(time.time()))
    contents = '''
#ifndef Zz_verinfo_h
#define Zz_verinfo_h

#define ZZ_REVISION "%s"
#define ZZ_PATH "%s"
#define ZZ_BUILD_TIME "%s"
#define ZZ_TAG "%s"

#endif
''' % (revision, path, build_time, tag)

    with open('%s/verinfo.h' % version_file_path, 'wb') as f:
        f.write(contents.encode())
        f.flush()

def remove_cmake_files(path):
    cmake_files = path + '/CMakeFiles'
    if os.path.exists(cmake_files):
        shutil.rmtree(cmake_files)

    make_files = path + '/Makefile'
    if os.path.isfile(make_files):
        os.remove(make_files)

    cmake_cache = path + '/CMakeCache.txt'
    if os.path.isfile(cmake_cache):
        os.remove(cmake_cache)

    for f in glob.glob(path + '/*.a'):
        os.remove(f)
    for f in glob.glob(path + '/*.so'):
        os.remove(f)


def clean(path, incremental=False):
    if not incremental:
        for fpath, dirs, fs in os.walk(path):
            remove_cmake_files(fpath)

    if not os.path.exists(path):
        os.makedirs(path)

def libtool_libs(src_libs, dst_lib):
    src_lib_str = ''
    for l in src_libs:
        src_lib_str = '%s %s'%(src_lib_str, l)

    print(src_lib_str)
    ret = os.system('libtool -static -o %s %s' %(dst_lib, src_lib_str))
    if ret != 0:
        print('!!!!!!!!!!!libtool %s fail!!!!!!!!!!!!!!!' %(dst_lib))
        return False

    return True

def lipo_libs(src_libs, dst_lib):
    src_lib_str = u''
    for l in src_libs:
        src_lib_str = '%s %s'%(src_lib_str, l)

    cmd = 'lipo -create %s -output %s' %(src_lib_str, dst_lib)
    ret = os.system(cmd)
    if ret != 0:
        print('!!!!!!!!!!!lipo_libs %s fail, cmd:%s!!!!!!!!!!!!!!!' %(dst_lib, cmd))
        return False

    return True

def copy_file(src, dst):
    if not os.path.isfile(src):
        print('Warning: %s not exist' %(src))
        return;

    if dst.rfind("/") != -1 and not os.path.exists(dst[:dst.rfind("/")]):
        os.makedirs(dst[:dst.rfind("/")])

    shutil.copy(src, dst)

def make_static_framework(src_lib, dst_framework, header_file_mappings, header_files_src_base='./'):
    if os.path.exists(dst_framework):
        shutil.rmtree(dst_framework)

    os.makedirs(dst_framework)
    shutil.copy(src_lib, dst_framework)

    framework_path = dst_framework + '/Headers'
    for (src, dst) in header_file_mappings.items():
        pos = src.rfind("/")
        if pos == -1:
            file_name = src
        else:
            file_name = src[pos:]

        if dst == '':
            copy_file(header_files_src_base + src, framework_path + '/' + file_name)
        else:
            copy_file(header_files_src_base + src, framework_path + "/" + dst + '/' + file_name)

    return True
