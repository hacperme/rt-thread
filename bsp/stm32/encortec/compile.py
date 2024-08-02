import sys
import os
import subprocess
from tools.fw_merge import merge_bin_files
from tools.export_api_addr import export_api_addr

ENCORTEC_BL_MAP_NAME = "./bootloader/encortec-bootloader.map"
ENCORTEC_BL_BIN_NAME = "./bootloader/encortec-bootloader.bin"
ENCORTEC_APP_BIN_NAME = "./application/build/encortec-application.bin"
ENCORTEC_FW_NAME = "./encortec-fw-all.bin"
ENCORTEC_BL_API_LIST_NAME = "./tools/api_list.txt"
ENCORTEC_BL_API_ADDR_EXPORT_NAME = "./application/rt_api/rt_api_addr.h"

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

def merge_fw():
    print("------ Merging Bootloader and App...")
    merge_bin_files('./bootloader/encortec-bootloader.bin', './application/build/encortec-application.bin', ENCORTEC_FW_NAME)
    print("------ Done!")

def clean_merged_fw():
    print(f"------ Cleaning {ENCORTEC_FW_NAME}...")
    if os.path.exists(ENCORTEC_FW_NAME):
        os.remove(ENCORTEC_FW_NAME)
    print("------ Done!")

def export_api_addr_list():
    print("------ Exporting Bootloader API addresses...")
    export_api_addr(ENCORTEC_BL_MAP_NAME, ENCORTEC_BL_API_LIST_NAME, ENCORTEC_BL_API_ADDR_EXPORT_NAME)
    print("------ Done!")

def clean_api_addr_list():
    print(f"------ Cleaning {ENCORTEC_BL_API_ADDR_EXPORT_NAME}...")
    if os.path.exists(ENCORTEC_BL_API_ADDR_EXPORT_NAME):
        os.remove(ENCORTEC_BL_API_ADDR_EXPORT_NAME)
    print("------ Done!")

def build_all():
    build_bootloader()
    export_api_addr_list()
    build_app()
    merge_fw()

def clean_all():
    clean_bootloader()
    clean_app()
    clean_merged_fw()
    clean_api_addr_list()

def print_usage(with_error=False):
    print(("Invalid command. " if with_error else "") + "Please use one of the following commands:")
    print("1. python ./compile.py -a|--app [-c|--clean] # Compile App code")
    print("2. python ./compile.py -b|--bootloader [-c|--clean] # Compile Bootloader code")
    print("3. python ./compile.py [-A|--all] [-c|--clean] # Compile Bootloader and then App")
    print("4. python ./compile.py -m|--merge [-c|--clean] # Merge Bootloader and App firmware")
    print("4. python ./compile.py -e|--export-api-addr [-c|--clean] # Export Bootloader API addresses")
    print("5. python ./compile.py -h|--help # Print this message")


"""
python .\compile.py

python .\compile.py -c|--clean
python .\compile.py -A|--all
python .\compile.py -a|--app
python .\compile.py -b|--bootloader
python .\compile.py -e|--export-api-addr
python .\compile.py -m|--merge
python .\compile.py -h|--help

python .\compile.py -A|--all -c|--clean
python .\compile.py -a|--app -c|--clean
python .\compile.py -b|--bootloader -c|--clean
python .\compile.py -e|--export-api-addr -c|--clean
python .\compile.py -m|--merge -c|--clean
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
        elif sys.argv[1] in ['-e', '--export-api-addr']:
            export_api_addr_list()
        elif sys.argv[1] in ['-m', '--merge']:
            merge_fw()
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
            elif sys.argv[1] in ['-e', '--export-api-addr']:
                clean_api_addr_list()
            elif sys.argv[1] in ['-m', '--merge']:
                clean_merged_fw()
            else:
                print_usage(True)
    else:
        print_usage(True)