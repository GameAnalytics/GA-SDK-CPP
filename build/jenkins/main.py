#!/usr/bin/python
from install_dependencies import install_dependencies
import config
import sys
import getopt
import os
from build import main as build_main, valid_target_names

sys.path.append(os.path.abspath(os.path.join(__file__, '..', '..')))

# import subprocess
# from sys import platform


def build(specific_target=None, silent=False, vs="2017", skip_tizen=False):
    try:
        os.makedirs(config.BUILD_DIR)
    except OSError:
        pass

    try:
        os.makedirs(config.PACKAGING_OUTPUT_DIRECTORY)
    except OSError:
        pass

    os.chdir('..')
    if specific_target:
        if skip_tizen:
            build_main(['-t', specific_target, '-v', vs, "-n"], silent=silent)
        else:
            build_main(['-t', specific_target, '-v', vs], silent=silent)
    else:
        if skip_tizen:
            build_main(['-v', vs, "-n"], silent=silent)
        else:
            build_main(['-v', vs], silent=silent)

def print_help():
    print 'main.py -t <target> --skip-dependencies --help'
    print 'Valid targets:'
    for target in valid_target_names:
        print '  ' + target


# @timing - bencharking build
# -t / --target: [specific target] (default all is built)
def main(argv):
    try:
        opts, args = getopt.getopt(argv, "t:h:v:ns", ["target=", "skip-dependencies", "help", "vs", "notizen", "silent"])
    except getopt.GetoptError:
        print_help()
        sys.exit(2)

    build_dependencies = True
    build_target_name = None
    silent = False
    visual_studio = "2017"
    installTizen = False
    skip_tizen = False

    for opt, arg in opts:
        if opt in ['-h', '--help']:
            print_help()
            sys.exit()
        elif opt in ['-t', '--target']:
            if arg in valid_target_names:
                build_target_name = arg
            else:
                print "ERROR: target name specified not in valid names: " + arg
        elif opt in ['--skip-dependencies']:
            print "!! SKIPPING DEPENDENCIES INSTALLATION !!"
            build_dependencies = False
        elif opt in ['-s', '--silent']:
            print "RUNNING EVERYTHING AS SILENTLY AS POSSIBLE"
            silent = True
        elif opt in ['-v', '--vs']:
            visual_studio = arg
        elif opt in ['-n', '--notizen']:
            print "SKIPPING TIZEN INSTALLATION"
            skip_tizen = True

    os.chdir(config.BUILD_ROOT)
    if build_target_name is not None:
        installTizen = 'tizen' in build_target_name
    else:
        installTizen = True

    if skip_tizen is True:
        installTizen = False

    if build_dependencies is True:
        install_dependencies(installTizen=installTizen, silent=silent)

    if build_target_name is not None:
        build(specific_target=build_target_name, silent=silent, vs=visual_studio, skip_tizen=skip_tizen)
    else:
        # build all
        build(silent=silent, vs=visual_studio, skip_tizen=skip_tizen)

if __name__ == '__main__':
    main(sys.argv[1:])
