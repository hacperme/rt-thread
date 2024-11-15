import sys
import os
import subprocess
from datetime import datetime
from tools.fw_merge import merge_bin_files
from tools.export_api_addr import export_api_addr
from tools.gen_rt_api_c import gen_rt_api_c
from tools.flash import flash

ENCORTEC_BL_MAP_NAME = "./bootloader/encortec-bootloader.map"
ENCORTEC_BL_BIN_NAME = "./bootloader/encortec-bootloader.bin"
ENCORTEC_APP_BIN_NAME = "./application/build/encortec-application-%s.bin"
ENCORTEC_FW_NAME = "./encortec-fw-all.bin"
ENCORTEC_BL_API_LIST_NAME = "./tools/api_list.txt"
ENCORTEC_BL_API_ADDR_EXPORT_NAME = "./application/rt_api/rt_api_addr.h"
ENCORTEC_APP_API_TYPEDEF_H_NAME = "./application/rt_api/rt_api_typedef.h"
ENCORTEC_APP_API_C_NAME = "./application/rt_api/rt_api.c"
ENCORTEC_APP_PART = "A"
ENCORTEC_APP_VERSION = "ENCORTECSTM32R01M02A01"
ENCORTEC_APP_SUBEDITION = "V01"
ENCORTEC_APP_BUILD_TIME = ""


def build_bootloader():
    print("------ Building Bootloader...")
    subprocess.run("cd ./bootloader && scons && cd ..", shell=True)
    print("------ Done!")


def clean_bootloader():
    print("------ Cleaning Bootloader...")
    subprocess.run("cd ./bootloader && scons -c && cd ..", shell=True)
    print("------ Done!")


def set_app_version():
    print("------ Seting Application Version...")
    for app_part in ("A", "B"):
        if ENCORTEC_APP_PART.find(app_part) != -1:
            print("write version to", (ENCORTEC_APP_BIN_NAME % app_part.lower()))
            with open(ENCORTEC_APP_BIN_NAME % app_part.lower(), "rb+") as f:
                f.seek(40, 0)
                if (f.write(ENCORTEC_APP_VERSION.encode()) == len(ENCORTEC_APP_VERSION.encode())):
                    print("ENCORTEC_APP_VERSION", ENCORTEC_APP_VERSION.encode())
                else:
                    print("Write ENCORTEC_APP_VERSION failed")
                f.seek(104, 0)
                if (f.write(ENCORTEC_APP_SUBEDITION.encode()) == len(ENCORTEC_APP_SUBEDITION.encode())):
                    print("ENCORTEC_APP_SUBEDITION", ENCORTEC_APP_SUBEDITION.encode())
                else:
                    print("Write ENCORTEC_APP_SUBEDITION failed")
                f.seek(112, 0)
                ENCORTEC_APP_BUILD_TIME = datetime.utcnow().isoformat()
                if (f.write(ENCORTEC_APP_BUILD_TIME.encode()) == len(ENCORTEC_APP_BUILD_TIME.encode())):
                    print("ENCORTEC_APP_BUILD_TIME", ENCORTEC_APP_BUILD_TIME.encode())
                else:
                    print("Write ENCORTEC_APP_BUILD_TIME failed")
                f.flush()
    print("------ Done!")


def build_app():
    print("------ Building Application...")
    for app_part in ("A", "B"):
        if ENCORTEC_APP_PART.find(app_part) != -1:
            subprocess.run(f"cd ./application && set APP_PART={app_part} && scons && cd ..", shell=True)
    print("------ Done!")
    set_app_version()


def clean_app():
    print("------ Cleaning Application...")
    subprocess.run("cd ./application && scons -c && cd ..", shell=True)
    print("------ Done!")


def merge_fw():
    app_a_file = ENCORTEC_APP_BIN_NAME % "A" if ENCORTEC_APP_PART.find("A") != -1 else None
    app_b_file = ENCORTEC_APP_BIN_NAME % "B" if ENCORTEC_APP_PART.find("B") != -1 else None
    msg_a = "part of A" if app_a_file else ""
    msg_b = "part of B" if app_b_file else ""
    msg = f"{msg_a} and {msg_b}" if msg_a and msg_b else (msg_a if msg_a else msg_b)
    print(f"------ Merging Bootloader and App {msg}...")
    merge_bin_files(ENCORTEC_BL_BIN_NAME, app_a_file, app_b_file, ENCORTEC_FW_NAME)
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


def gen_rt_api_c_file():
    print("------ Generating RT API C file...")
    gen_rt_api_c(ENCORTEC_APP_API_TYPEDEF_H_NAME, ENCORTEC_APP_API_C_NAME)
    print("------ Done!")


def clean_rt_api_c_file():
    print(f"------ Cleaning {ENCORTEC_APP_API_C_NAME}...")
    if os.path.exists(ENCORTEC_APP_API_C_NAME):
        os.remove(ENCORTEC_APP_API_C_NAME)
    print("------ Done!")


def build_all():
    build_bootloader()
    export_api_addr_list()
    gen_rt_api_c_file()
    build_app()
    merge_fw()


def clean_all():
    clean_bootloader()
    clean_app()
    clean_merged_fw()
    clean_api_addr_list()
    clean_rt_api_c_file()


def flash_fw():
    print("------ Flashing firmware to board...")
    openocd_path = os.getenv("OPENOCD_PATH")
    if openocd_path is None:
        print("OPENOCD_PATH environment variable is not set!")
        return
    flash(openocd_path, ENCORTEC_FW_NAME)
    print("------ Done!")


def print_usage(with_error=False):
    print(("Invalid command. " if with_error else "") + "Please use one of the following commands:")
    print("1. python ./compile.py -a|--app [-c|--clean] # Compile App code")
    print("1. python ./compile.py -a|--app [-p|--part_of_app] # Compile App part of A or B or AB (Default A)")
    print("2. python ./compile.py -b|--bootloader [-c|--clean] # Compile Bootloader code")
    print("3. python ./compile.py [-A|--all] [-c|--clean] # Compile Bootloader and then App part of A or B or AB (Default A)")
    print("3. python ./compile.py [-A|--all] [-p|--part_of_app] # Compile Bootloader and then App")
    print("4. python ./compile.py -m|--merge [-c|--clean] # Merge Bootloader and App firmware")
    print("4. python ./compile.py -m|--merge [-p|--part_of_app] # Merge Bootloader and App part of A or B or AB firmware (Default A)")
    print("5. python ./compile.py -f|--flash # Flash firmware")
    print("6. python ./compile.py -e|--export-api-addr [-c|--clean] # Export Bootloader API addresses")
    print("7. python ./compile.py -g|--gen-api [-c|--clean] # Export Bootloader API addresses")
    print("8. python ./compile.py -h|--help # Print this message")


"""
python .\compile.py

python .\compile.py -c|--clean
python .\compile.py -A|--all
python .\compile.py -a|--app
python .\compile.py -b|--bootloader
python .\compile.py -e|--export-api-addr
python .\compile.py -g|--gen-api
python .\compile.py -m|--merge
python .\compile.py -f|--flash
python .\compile.py -h|--help
python .\compile.py -v|--version

python .\compile.py -A|--all -c|--clean
python .\compile.py -a|--app -c|--clean
python .\compile.py -b|--bootloader -c|--clean
python .\compile.py -e|--export-api-addr -c|--clean
python .\compile.py -g|--gen-api -c|--clean
python .\compile.py -m|--merge -c|--clean

python .\compile.py -A|--all -p|--part_of_app
python .\compile.py -a|--app -p|--part_of_app
python .\compile.py -m|--merge -p|--part_of_app
python .\compile.py -A|--all -v|--version
python .\compile.py -a|--app -v|--version

python .\compile.py -A|--all -p|--part_of_app -v|--version
python .\compile.py -a|--app -p|--part_of_app -v|--version
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
        elif sys.argv[1] in ['-g', '--gen-api']:
            gen_rt_api_c_file()
        elif sys.argv[1] in ['-m', '--merge']:
            merge_fw()
        elif sys.argv[1] in ['-f', '--flash']:
            flash_fw()
        elif sys.argv[1] in ['-h', '--help']:
            print_usage()
        else:
            print_usage(True)
    elif len(sys.argv) == 3:
        if sys.argv[2] in ['-c', '--clean']:
            if sys.argv[1] in ['-A', '--all']:
                clean_all()
            elif sys.argv[1] in ['-a', '--app']:
                clean_app()
            elif sys.argv[1] in ['-b', '--bootloader']:
                clean_bootloader()
            elif sys.argv[1] in ['-e', '--export-api-addr']:
                clean_api_addr_list()
            elif sys.argv[1] in ['-g', '--gen-api']:
                clean_rt_api_c_file()
            elif sys.argv[1] in ['-m', '--merge']:
                clean_merged_fw()
            else:
                print_usage(True)
        else:
            if sys.argv[1] in ['-v', '--version']:
                versions = sys.argv[2].strip().upper().split("-")
                ENCORTEC_APP_VERSION = versions[0]
                if len(versions) > 1:
                    ENCORTEC_APP_SUBEDITION = versions[1]
                build_all()
            else:
                print_usage(True)
    elif len(sys.argv) == 4:
        if sys.argv[2] in ['-p', '--part_of_app']:
            if sys.argv[3].upper() not in ("A", "B", "AB"):
                ENCORTEC_APP_PART = sys.argv[3].strip().upper()
                if sys.argv[1] in ['-a', '--app']:
                    build_app()
                elif sys.argv[1] in ['-m', '--merge']:
                    merge_fw()
                elif sys.argv[1] in ['-A', '--all']:
                    build_all()
                else:
                    print_usage(True)
            else:
                print_usage(True)
        if sys.argv[2] in ['-v', '--version']:
            versions = sys.argv[3].strip().upper().split("-")
            ENCORTEC_APP_VERSION = versions[0]
            if len(versions) > 1:
                ENCORTEC_APP_SUBEDITION = versions[1]
            if sys.argv[1] in ['-a', '--app']:
                build_app()
            elif sys.argv[1] in ['-A', '--all']:
                build_all()
            else:
                print_usage(True)
        else:
            print_usage(True)
    else:
        print_usage(True)