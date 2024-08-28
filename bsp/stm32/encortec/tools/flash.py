import os
def flash(openocd_path, fw_bin_file):
    cmd = f'{openocd_path}/bin/openocd \
-f {openocd_path}/share/openocd/scripts/interface/jlink.cfg \
-c "transport select swd" \
-f {openocd_path}/share/openocd/scripts/target/stm32u5x.cfg \
-c "init" -c "halt" -c "flash write_image erase {fw_bin_file} 0x08000000" -c "reset" -c "shutdown"'
    print(cmd)
    os.system(cmd)