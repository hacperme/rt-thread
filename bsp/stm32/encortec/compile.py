import sys
import os
import subprocess
from tools.fw_merge import merge_bin_files

def build_bootloader():
    print("------ Building Bootloader...")
    subprocess.run("cd ./bootloader && scons -j8 && cd ..", shell=True)
    print("------ Done!")

def clean_bootloader():
    print("------ Cleaning Bootloader...")
    subprocess.run("cd ./bootloader && scons -c && cd ..", shell=True)
    print("------ Done!")

def build_app():
    print("------ Building Application...")
    subprocess.run("cd ./application && scons -j8 && cd ..", shell=True)
    print("------ Done!")

def clean_app():
    print("------ Cleaning Application...")
    subprocess.run("cd ./application && scons -c && cd ..", shell=True)
    print("------ Done!")

def build_all():
    build_bootloader()
    build_app()
    merge_bin_files('./bootloader/encortec-bootloader.bin', './application/build/encortec-application.bin', './encortec-fw-all.bin')

def clean_all():
    clean_bootloader()
    clean_app()
    print("------ Cleaning encortec-fw-all.bin...")
    if os.path.exists('./encortec-fw-all.bin'):
        os.remove('./encortec-fw-all.bin')
    print("------ Done!")

def print_usage(with_error=False):
    print(("Invalid command. " if with_error else "") + "Please use one of the following commands:")
    print("1. python ./compile.py -a|--app [-c|--clean] # Compile App code")
    print("2. python ./compile.py -b|--bootloader [-c|--clean] # Compile Bootloader code")
    print("3. python ./compile.py [-A|--all] [-c|--clean] # Compile Bootloader and then App")
    print("3. python ./compile.py -h|--help # Print this message")


"""
python .\compile.py

python .\compile.py -c|--clean
python .\compile.py -A|--all
python .\compile.py -a|--app
python .\compile.py -b|--bootloader
python .\compile.py -h|--help

python .\compile.py -A|--all -c|--clean
python .\compile.py -a|--app -c|--clean
python .\compile.py -b|--bootloader -c|--clean
"""
if __name__ == "__main__":
    if len(sys.argv) == 1:
        build_all()
    elif len(sys.argv) == 2:
        if sys.argv[1] in ['-c', '--clean']:
            clean_all()
        elif sys.argv[1] in ['-A', '--all']:
            build_all()
        elif sys.argv[1] in ['-a', '--app']:
            build_app()
        elif sys.argv[1] in ['-b', '--bootloader']:
            build_bootloader()
        elif sys.argv[1] in ['-h', '--help']:
            print_usage()
        else:
            print_usage(True)
    elif len(sys.argv) == 3:
        if sys.argv[2] not in ['-c', '--clean']:
            print_usage(True)
        else:
            if sys.argv[1] in ['-A', '--all']:
                clean_all()
            elif sys.argv[1] in ['-a', '--app']:
                clean_app()
            elif sys.argv[1] in ['-b', '--bootloader']:
                clean_bootloader()
            else:
                print_usage(True)
    else:
        print_usage(True)