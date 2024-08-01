import sys
import subprocess
from tools.fw_merge import merge_bin_files

def compile_bootloader():
    subprocess.run(["cd", "./bootloader"], shell=True)
    subprocess.run(["scons", "-j8"], shell=True)
    subprocess.run(["cd", ".."], shell=True)

def compile_app():
    subprocess.run(["cd", "./application"], shell=True)
    subprocess.run(["scons", "-j8"], shell=True)
    subprocess.run(["cd", ".."], shell=True)

if len(sys.argv) == 1 or sys.argv[1] in ['-A', '--all']:
    compile_bootloader()
    compile_app()
    merge_bin_files('./bootloader/encortec-bootloader.bin', './application/build/encortec-application.bin', './fw_all.bin')
elif sys.argv[1] in ['-b', '--bootloader']:
    compile_bootloader()
elif sys.argv[1] in ['-a', '--app']:
    compile_app()
else:
    print("Invalid command. Please use one of the following commands:")
    print("1. python ./compile.py -a|--app  # Compile App code")
    print("2. python ./compile.py -b|--bootloader  # Compile Bootloader code")
    print("3. python ./compile.py [-A|--all]  # Compile Bootloader and then App")
