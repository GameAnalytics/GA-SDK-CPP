#!/usr/bin/python
import os
import subprocess
import shutil


BUILD_ROOT = os.path.abspath(os.path.join(__file__, '..'))
BUILD_DIR = os.path.join(BUILD_ROOT, "build")


def make_test_build_dir():
    try:
        os.makedirs(BUILD_DIR)
    except OSError:
        pass

def remove_test_build_dir():
    shutil.rmtree(BUILD_DIR)

def change_to_build_dir():
    os.chdir(BUILD_DIR)


def run_cmake_tests():
    cmake_call = 'cmake -DPLATFORM:STRING=osx-static -G Xcode ..'
    subprocess.call(cmake_call, shell=True)


def compile_and_run_tests():
    run_tests_call = 'xcodebuild -project GameAnalyticsTests.xcodeproj -target GameAnalyticsTests -configuration Debug && ./Debug/GameAnalyticsTests'
    subprocess.call(run_tests_call, shell=True)


def main():
    remove_test_build_dir()
    make_test_build_dir()
    change_to_build_dir()
    run_cmake_tests()
    compile_and_run_tests()

if __name__ == '__main__':
    main()
