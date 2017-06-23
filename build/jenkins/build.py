#!/usr/bin/python

# This script extract the neccessary tools from the package folder,
# calls cmake to generate project files and builds them
import os
import platform
import sys
import getopt
import subprocess
import re
import config as Config
import libs.tools as LibTools
import shutil
import fileinput

if platform.system() == 'Windows':
    from _winreg import *

if os.name == "nt":
    __CSL = None
    def symlink(source, link_name):
        '''symlink(source, link_name)
           Creates a symbolic link pointing to source named link_name'''
        global __CSL
        if __CSL is None:
            import ctypes
            csl = ctypes.windll.kernel32.CreateSymbolicLinkW
            csl.argtypes = (ctypes.c_wchar_p, ctypes.c_wchar_p, ctypes.c_uint32)
            csl.restype = ctypes.c_ubyte
            __CSL = csl
        flags = 0
        if source is not None and os.path.isdir(source):
            flags = 1
        if __CSL(link_name, source, flags) == 0:
            raise ctypes.WinError()

    os.symlink = symlink

def call_process(process_arguments, process_workingdir, silent=False, useOutput=False):
    print('Call process ' + str(process_arguments) + ' in workingdir ' + process_workingdir)
    current_workingdir = os.getcwd()

    if not LibTools.folder_exists(process_workingdir):
        os.makedirs(process_workingdir)

    output = ''
    os.chdir(process_workingdir)
    if silent is True:
        if useOutput is True:
            output = subprocess.check_output(process_arguments, stdout=open(os.devnull, 'wb')).strip()
        else:
            subprocess.check_call(process_arguments, stdout=open(os.devnull, 'wb'))
    else:
        if useOutput is True:
            output = subprocess.check_output(process_arguments).strip()
        else:
            subprocess.check_call(process_arguments)

    os.chdir(current_workingdir)

    return output


class Target(object):
    def __init__(self, name):
        self.name = name


class TargetCMake(Target):
    def __init__(self, name, generator):
        super(TargetCMake, self).__init__(name)
        self.generator = generator

    def build_dir(self):
        return os.path.abspath(os.path.join(__file__, '..', 'build', self.name))

    def create_project_file(self):
        call_process(
            [
                os.path.join(
                    Config.CMAKE_ROOT,
                    'bin',
                    'cmake'
                ),
                '../../../cmake/gameanalytics/',
                '-DPLATFORM:STRING=' + self.name,
                '-G',
                self.generator
            ],
            self.build_dir()
        )

    def build(self, silent=False):
        raise NotImplemented()


class TargetOSX(TargetCMake):
    def build(self, silent=False):
        call_process(
            [
                'xcodebuild',
                '-configuration',
                'Release'
            ],
            self.build_dir(),
            silent=silent
        )

        call_process(
            [
                'xcodebuild',
                '-configuration',
                'Debug'
            ],
            self.build_dir(),
            silent=silent
        )

        self.binary_name = {
            'osx-shared': 'libGameAnalytics.dylib',
            'osx-static': 'libGameAnalytics.a',
        }[self.name]
        self.target_name = {
            'osx-shared': 'GameAnalytics.bundle',
            'osx-static': 'libGameAnalytics.a',
        }[self.name]

        debug_dir = os.path.abspath(os.path.join(__file__, '..', '..', '..', 'export', self.name, 'Debug'))
        release_dir = os.path.abspath(os.path.join(__file__, '..', '..', '..', 'export', self.name, 'Release'))

        # remove folders if there
        LibTools.remove_folder(debug_dir)
        LibTools.remove_folder(release_dir)

        if not LibTools.folder_exists(debug_dir):
            os.makedirs(debug_dir)

        if not LibTools.folder_exists(release_dir):
            os.makedirs(release_dir)

        shutil.copy(
            os.path.join(self.build_dir(), 'Debug', self.binary_name),
            os.path.join(debug_dir, self.binary_name)
        )

        shutil.move(
            os.path.join(self.build_dir(), 'Debug', self.binary_name),
            os.path.join(debug_dir, self.target_name)
        )

        shutil.copy(
            os.path.join(self.build_dir(), 'Release', self.binary_name),
            os.path.join(release_dir, self.binary_name)
        )

        shutil.move(
            os.path.join(self.build_dir(), 'Release', self.binary_name),
            os.path.join(release_dir, self.target_name)
        )

class TargetWin(TargetCMake):
    @staticmethod
    def get_msbuild_path():
        try:
            aReg = ConnectRegistry(None, HKEY_LOCAL_MACHINE)
            aKey = OpenKey(aReg, r'SOFTWARE\Microsoft\MSBuild\ToolsVersions\14.0')
            return QueryValueEx(aKey, "MSBuildToolsPath")[0]
        except OSError:
            print 'msbuild path not found'
        return ''

        path = call_process(
            [
                os.path.join(
                    Config.BUILD_ROOT,
                    'vswhere'
                ),
                '-requires',
                'Microsoft.Component.MSBuild',
                '-property',
                'installationPath'
            ],
            os.getcwd(),
            useOutput=True
        )

        path = os.path.join(path, "MSBuild", "15.0", "Bin")

        return path

    def build(self, silent=False):
        # call msbuild and compile projects in solution
        subprocess.check_call([
            os.path.join(self.get_msbuild_path(), 'msbuild.exe'),
            os.path.join(self.build_dir(), 'GameAnalytics.sln'),
            '/m',  # parallel builds
            '/t:GameAnalytics',
            '/p:Configuration=Release',
        ])

        subprocess.check_call([
            os.path.join(self.get_msbuild_path(), 'msbuild.exe'),
            os.path.join(self.build_dir(), 'GameAnalytics.sln'),
            '/m',  # parallel builds
            '/t:GameAnalytics',
            '/p:Configuration=Debug',
        ])

        debug_dir = os.path.abspath(os.path.join(__file__, '..', '..', '..', 'export', self.name, 'Debug'))
        release_dir = os.path.abspath(os.path.join(__file__, '..', '..', '..', 'export', self.name, 'Release'))

        # remove folders if there
        LibTools.remove_folder(debug_dir)
        LibTools.remove_folder(release_dir)

        shutil.move(
            os.path.join(self.build_dir(), 'Debug'),
            debug_dir
        )

        shutil.move(
            os.path.join(self.build_dir(), 'Release'),
            release_dir
        )

class TargetWin10(TargetWin):
    def create_project_file(self):
        call_process(
            [
                os.path.join(
                    Config.CMAKE_ROOT,
                    'bin',
                    'cmake'
                ),
                '../../../cmake/gameanalytics/',
                '-DPLATFORM:STRING=' + self.name,
                '-DCMAKE_SYSTEM_NAME=WindowsStore',
                '-DCMAKE_SYSTEM_VERSION=10.0',
                '-G',
                self.generator
            ],
            self.build_dir()
        )

class TargetTizen(TargetCMake):
    def create_project_file(self):
        build_folder = os.path.join(Config.BUILD_DIR, self.name)

        if sys.platform == 'darwin':
            tizen_ide = os.path.join(Config.TIZEN_ROOT, "tools", "ide", "bin", "tizen.sh")
        else:
            tizen_ide = os.path.join(Config.TIZEN_ROOT, "tools", "ide", "bin", "tizen.bat")

        tizen_src_dir = os.path.join(build_folder, "src")
        tizen_include_dir = os.path.join(build_folder, "inc")

        if LibTools.folder_exists(build_folder):
            if sys.platform != 'darwin':
                if LibTools.folder_exists(tizen_include_dir):
                    os.rmdir(tizen_include_dir)
                if LibTools.folder_exists(tizen_src_dir):
                    os.rmdir(tizen_src_dir)
            shutil.rmtree(build_folder)

        libType = 'StaticLibrary'
        lib_type = 'staticLib'
        if 'shared' in self.name:
            libType = 'SharedLibrary'
            lib_type = 'sharedLib'

        call_process(
            [
                tizen_ide,
                'create',
                'native-project',
                '-p',
                'mobile-2.4',
                '-t',
                libType,
                '-n',
                self.name,
                '--',
                Config.BUILD_DIR
            ],
            Config.BUILD_DIR
        )

        src_dir = os.path.abspath(os.path.join(__file__, '..', '..', '..', 'source'))
        dependencies_dir = os.path.join(src_dir, "dependencies")
        project_def_tmp = os.path.abspath(os.path.join(__file__, '..', '..', 'tizen', 'project_def_tmp.prop'))
        project_def = os.path.join(build_folder, 'project_def.prop')

        shutil.rmtree(tizen_src_dir)
        shutil.rmtree(tizen_include_dir)

        os.symlink(src_dir, tizen_src_dir)
        os.symlink(dependencies_dir, tizen_include_dir)

        shutil.copy(project_def_tmp, project_def)

        for line in fileinput.input(project_def, inplace=True):
            if '<LIB_TYPE>' in line:
                line = line.replace('<LIB_TYPE>', lib_type)
            if '<ASYNC>' in line:
                if self.generator == 'x86':
                    line = line.replace('<ASYNC>', 'NO_ASYNC')
                else:
                    line = line.replace('<ASYNC>', '')
            sys.stdout.write(line)

        flags_file = os.path.join(build_folder, 'Build', 'flags.mk')

        with open(flags_file, 'r') as file:
            lines = file.readlines()

        for index, line in enumerate(lines):
            if line.startswith('CPP_COMPILE_FLAGS'):
                lines[index] = line.strip() + " -std=c++11\n"

        with open(flags_file, 'w') as file:
            file.writelines(lines)

    def build(self, silent=False):
        build_folder = os.path.join(Config.BUILD_DIR, self.name)
        if sys.platform == 'darwin':
            tizen_ide = os.path.join(Config.TIZEN_ROOT, "tools", "ide", "bin", "tizen.sh")
        else:
            tizen_ide = os.path.join(Config.TIZEN_ROOT, "tools", "ide", "bin", "tizen.bat")

        compiler = 'gcc'

        call_process(
            [
                tizen_ide,
                'build-native',
                '-a',
                self.generator,
                '-c',
                compiler,
                '-C',
                'Release',
                '--',
                build_folder
            ],
            self.build_dir(),
            silent=silent
        )

        call_process(
            [
                tizen_ide,
                'build-native',
                '-a',
                self.generator,
                '-c',
                compiler,
                '-C',
                'Debug',
                '--',
                build_folder
            ],
            self.build_dir(),
            silent=silent
        )

        libEnding = 'a'
        if 'shared' in self.name:
            libEnding = 'so'

        debug_file = os.path.abspath(os.path.join(__file__, '..', '..', '..', 'export', self.name, 'Debug', 'libGameAnalytics.' + libEnding))
        release_file = os.path.abspath(os.path.join(__file__, '..', '..', '..', 'export', self.name, 'Release', 'libGameAnalytics.' + libEnding))

        if not os.path.exists(os.path.dirname(debug_file)):
            os.makedirs(os.path.dirname(debug_file))

        if not os.path.exists(os.path.dirname(release_file)):
            os.makedirs(os.path.dirname(release_file))

        shutil.move(
            os.path.join(self.build_dir(), 'Debug', 'libGameAnalytics.' + libEnding),
            debug_file
        )

        shutil.move(
            os.path.join(self.build_dir(), 'Release', 'libGameAnalytics.' + libEnding),
            release_file
        )

class TargetLinux(TargetCMake):
    def __init__(self, name, generator, architecture):
        super(TargetLinux, self).__init__(name, generator)
        self.architecture = architecture

    def create_project_file(self):
        print 'Skip create_project_file for Linux'


    def build(self, silent=False):
	    # 32-bit libs
        call_process(
            [
                os.path.join(
                    Config.CMAKE_ROOT,
                    'bin',
                    'cmake'
                ),
                '../../../cmake/gameanalytics/',
                '-DPLATFORM:STRING=' + self.name,
                '-DCMAKE_BUILD_TYPE=RELEASE',
		        '-DTARGET_ARCH:STRING=' + self.architecture,
                '-G',
                self.generator
            ],
            self.build_dir()
        )

        call_process(
            [
                'make',
                'clean'
            ],
            self.build_dir(),
            silent=silent
        )

        call_process(
            [
                'make'
            ],
            self.build_dir(),
            silent=silent
        )

        call_process(
            [
                os.path.join(
                    Config.CMAKE_ROOT,
                    'bin',
                    'cmake'
                ),
                '../../../cmake/gameanalytics/',
                '-DPLATFORM:STRING=' + self.name,
                '-DCMAKE_BUILD_TYPE=DEBUG',
		        '-DTARGET_ARCH:STRING=' + self.architecture,
                '-G',
                self.generator
            ],
            self.build_dir()
        )

        call_process(
            [
                'make',
                'clean'
            ],
            self.build_dir(),
            silent=silent
        )

        call_process(
            [
                'make'
            ],
            self.build_dir(),
            silent=silent
        )

        libEnding = 'a'
        if 'shared' in self.name:
            libEnding = 'so'

        debug_file = os.path.abspath(os.path.join(__file__, '..', '..', '..', 'export', self.name, 'Debug', 'libGameAnalytics.' + libEnding))
        release_file = os.path.abspath(os.path.join(__file__, '..', '..', '..', 'export', self.name, 'Release', 'libGameAnalytics.' + libEnding))

        if not os.path.exists(os.path.dirname(debug_file)):
            os.makedirs(os.path.dirname(debug_file))

        if not os.path.exists(os.path.dirname(release_file)):
            os.makedirs(os.path.dirname(release_file))

        shutil.move(
            os.path.join(self.build_dir(), 'Debug', 'libGameAnalytics.' + libEnding),
            debug_file
        )

        shutil.move(
            os.path.join(self.build_dir(), 'Release', 'libGameAnalytics.' + libEnding),
            release_file
        )

all_targets = {
    'win32-vc140-static': TargetWin('win32-vc140-static', 'Visual Studio 14'),
    'win32-vc120-static': TargetWin('win32-vc120-static', 'Visual Studio 12'),
    'win32-vc140-shared': TargetWin('win32-vc140-shared', 'Visual Studio 14'),
    'win32-vc140-shared-nowmi': TargetWin('win32-vc140-shared-nowmi', 'Visual Studio 14'),
    'win32-vc120-shared': TargetWin('win32-vc120-shared', 'Visual Studio 12'),
    'win64-vc140-static': TargetWin('win64-vc140-static', 'Visual Studio 14 Win64'),
    'win64-vc120-static': TargetWin('win64-vc120-static', 'Visual Studio 12 Win64'),
    'win64-vc140-shared': TargetWin('win64-vc140-shared', 'Visual Studio 14 Win64'),
    'win64-vc120-shared': TargetWin('win64-vc120-shared', 'Visual Studio 12 Win64'),
    'uwp-x86-vc140-static': TargetWin10('uwp-x86-vc140-static', 'Visual Studio 15'),
	'uwp-x64-vc140-static': TargetWin10('uwp-x64-vc140-static', 'Visual Studio 15 Win64'),
	'uwp-arm-vc140-static': TargetWin10('uwp-arm-vc140-static', 'Visual Studio 15 ARM'),
    'uwp-x86-vc140-shared': TargetWin10('uwp-x86-vc140-shared', 'Visual Studio 15'),
	'uwp-x64-vc140-shared': TargetWin10('uwp-x64-vc140-shared', 'Visual Studio 15 Win64'),
	'uwp-arm-vc140-shared': TargetWin10('uwp-arm-vc140-shared', 'Visual Studio 15 ARM'),
    'osx-static': TargetOSX('osx-static', 'Xcode'),
    'osx-shared': TargetOSX('osx-shared', 'Xcode'),
    'tizen-arm-static': TargetTizen('tizen-arm-static', 'arm'),
    'tizen-arm-shared': TargetTizen('tizen-arm-shared', 'arm'),
    'tizen-x86-static': TargetTizen('tizen-x86-static', 'x86'),
    'tizen-x86-shared': TargetTizen('tizen-x86-shared', 'x86'),
    'linux-x86-static': TargetLinux('linux-x86-static', 'Unix Makefiles', '-m32'),
    'linux-x86-shared': TargetLinux('linux-x86-shared', 'Unix Makefiles', '-m32'),
    'linux-x64-static': TargetLinux('linux-x64-static', 'Unix Makefiles', '-m64'),
    'linux-x64-shared': TargetLinux('linux-x64-shared', 'Unix Makefiles', '-m64'),
}

available_targets = {
    'Darwin': {
        'osx-static': all_targets['osx-static'],
        'osx-shared': all_targets['osx-shared'],
        'tizen-arm-static': all_targets['tizen-arm-static'],
        'tizen-arm-shared': all_targets['tizen-arm-shared'],
        'tizen-x86-static': all_targets['tizen-x86-static'],
        'tizen-x86-shared': all_targets['tizen-x86-shared'],
    },
    'Windows': {
        'win32-vc140-static': all_targets['win32-vc140-static'],
        'win32-vc120-static': all_targets['win32-vc120-static'],
        'win32-vc140-shared': all_targets['win32-vc140-shared'],
        'win32-vc140-shared-nowmi': all_targets['win32-vc140-shared-nowmi'],
        'win32-vc120-shared': all_targets['win32-vc120-shared'],
        'win64-vc140-static': all_targets['win64-vc140-static'],
        'win64-vc120-static': all_targets['win64-vc120-static'],
        'win64-vc140-shared': all_targets['win64-vc140-shared'],
        'win64-vc120-shared': all_targets['win64-vc120-shared'],
        'uwp-x86-vc140-static': all_targets['uwp-x86-vc140-static'],
		'uwp-x64-vc140-static': all_targets['uwp-x64-vc140-static'],
		'uwp-arm-vc140-static': all_targets['uwp-arm-vc140-static'],
        'uwp-x86-vc140-shared': all_targets['uwp-x86-vc140-shared'],
		'uwp-x64-vc140-shared': all_targets['uwp-x64-vc140-shared'],
		'uwp-arm-vc140-shared': all_targets['uwp-arm-vc140-shared'],
        'tizen-arm-static': all_targets['tizen-arm-static'],
        'tizen-arm-shared': all_targets['tizen-arm-shared'],
        'tizen-x86-static': all_targets['tizen-x86-static'],
        'tizen-x86-shared': all_targets['tizen-x86-shared'],
    },
    'Linux': {
        'linux-x86-static': all_targets['linux-x86-static'],
        'linux-x86-shared': all_targets['linux-x86-shared'],
        'linux-x64-static': all_targets['linux-x64-static'],
        'linux-x64-shared': all_targets['linux-x64-shared'],
    }
}[platform.system()]

# Sorted since we want android-wrapper to be built after android-shared (due to jni generation)
valid_target_names = sorted(available_targets.keys())


def print_help():
    print 'build.py <list_of_targets>'
    print 'valid targets:'
    for target in valid_target_names:
        print '  ' + target


def build(target_name, silent=False):
    if target_name not in available_targets:
        print('target %(target_name)s not supported on this platform' % locals())
        sys.exit(1)

    target = available_targets[target_name]
    target.create_project_file()
    target.build(silent=silent)


def build_targets(target_names, silent=False):

    for target_name in target_names:
        print ""
        print "-----------------------------------------"
        print ""
        print "    BUILDING TARGET: " + target_name
        print ""
        print "-----------------------------------------"
        print ""

        build(target_name, silent=silent)


# @timing - benchmarking build
def main(argv, silent=False):
    print "-----------------------------------------"
    print "    BUILDING"
    print "-----------------------------------------"
    print "Build arguments..."
    print argv

    try:
        opts, args = getopt.getopt(argv, "t:h", ["target=", "help"])
    except getopt.GetoptError:
        print_help()
        sys.exit(2)

    build_target_name = None

    for opt, arg in opts:
        if opt in ('-h', '--help'):
            print_help()
            sys.exit()
        elif opt in ('-t', '--target'):
            if arg in valid_target_names:
                build_target_name = arg
            else:
                print "Target: " + arg + " is not part of allowed targets."
                print_help()
                sys.exit(2)

    if build_target_name:
        build_targets([arg], silent=silent)
    else:
        # build all
        build_targets(valid_target_names, silent=silent)

if __name__ == '__main__':
    main(sys.argv[1:])
