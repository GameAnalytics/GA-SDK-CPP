#!/usr/bin/python

import os
import sys
import urllib
import config
import zipfile
import tarfile
import shutil
import stat
import subprocess

from sys import platform

if platform == 'win32':  # win32 and/or win64
    CMAKE_URL = 'https://cmake.org/files/v3.5/cmake-3.5.2-win32-x86.zip'
elif platform == 'darwin':  # OSX
    CMAKE_URL = 'http://www.cmake.org/files/v3.2/cmake-3.2.2-Darwin-universal.tar.gz'
# elif platform in ('linux', 'linux2'):
else:
    raise NotImplementedError('platform %s is currently not supported' % platform)

cmake_package = os.path.join(config.BUILD_ROOT, CMAKE_URL.split('/')[-1])


def download(url, destination, silent=False):
    def reporthook(count, block_size, total_size):
        sys.stdout.write('{3}downloaded {0} bytes of {1} bytes ({2:.1f}%)               '.format(
            count*block_size,
            total_size,
            100.0 * count * block_size / total_size,
            '\r' * 80
        ))

    if silent is True:
        urllib.urlretrieve(url, destination)
    else:
        urllib.urlretrieve(url, destination, reporthook)
    print 'download done'

# windows has an restriction about the size of a filename (260)
# the android sdk contains some eclipse files, which are very long
# and cause a extraction error during a simple extraction
# therefore I wrote this function, which extracts the files if possible
# def unzip(zip_file_location, extract_location):
#   max_path_size = 240
#   skipped_files = 0
#   with zipfile.ZipFile(zip_file_location, 'r') as zf:
#       root_in_zip = zf.namelist()[0].split('/')[0]
#
#       print root_in_zip
#       sys.exit(1)
#
#       for archive_member in zf.infolist():
#           dst_path = os.path.join(extract_location, archive_member.filename)
#           if len(dst_path) > max_path_size:
#               skipped_files += 1
#               print('skipping: ' + archive_member.filename)
#           else:
#               print('extract: ' + archive_member.filename)
#               zf.extract(archive_member, extract_location)
#       shutil.move( root_in_zip, extract_location)
#
#   print('skipped files total: ' + str(skipped_files))


def unzip(package, dst, silent=False):
    print('unzipping "%s"' % package)
    with zipfile.ZipFile(package, 'r') as zf:
        root_in_zip = zf.namelist()[0].split('/')[0]
        zf.extractall()
        shutil.move(root_in_zip, dst)
    print('[done]')


def untar(package, dst, silent=False):
    print('untarring "%s"' % package)
    # with tarfile.TarFile(package, 'r') as zf:
    with tarfile.open(package, 'r') as zf:
        root_in_zip = zf.getnames()[0].split('/')[0]
        zf.extractall()
        shutil.move(root_in_zip, dst)
    print('[done]')


def uncompress(package, dst, silent=False):
    if '.tar' in package:
        untar(package, dst, silent=silent)
    else:
        unzip(package, dst, silent=silent)


def install_cmake(silent=False):
    if not os.path.exists(config.CMAKE_ROOT):
        print "-------------- CMAKE ---------------"
        if not os.path.exists(cmake_package):
            print "--> DOWNLOADING CMAKE"
            download(CMAKE_URL, cmake_package, silent=silent)
        print "--> UNCOMPRESSING CMAKE"
        uncompress(cmake_package, config.CMAKE_ROOT, silent=silent)

        if platform == 'darwin':
            print "--> MOVING CMAKE + CLEANUP"
            shutil.move(
                os.path.join(config.CMAKE_ROOT, 'CMake.app', 'Contents'),
                config.CMAKE_ROOT + '_'
            )
            shutil.rmtree(config.CMAKE_ROOT)
            shutil.move(config.CMAKE_ROOT + '_', config.CMAKE_ROOT)

        os.unlink(cmake_package)


def install_dependencies(silent=False):
    if silent is True:
        print "SILENT DEPENDENCY INSTALL"
    install_cmake(silent=silent)

if __name__ == '__main__':
    os.chdir(config.BUILD_ROOT)
    install_dependencies()
