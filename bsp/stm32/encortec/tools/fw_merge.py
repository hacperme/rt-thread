BOOTLOADER_SIZE = 512 * 1024 + 16 * 1024
APP_SIZE = 760 * 1024

def merge_bin_files(bootloader_file, app_a_file, app_b_file, output_file):
    with open(bootloader_file, 'rb') as bootloader_f:
        bootloader_data = bootloader_f.read()

    app_a_data = b""
    if app_a_file:
        with open(app_a_file, 'rb') as app_f:
            app_a_data = app_f.read()

    app_b_data = b""
    if app_b_file:
        with open(app_b_file, 'rb') as app_f:
            app_b_data = app_f.read()

    # Pad the bootloader data with 0xFF if its size is less than BOOTLOADER_SIZE
    if len(bootloader_data) < BOOTLOADER_SIZE:
        bootloader_data += b'\xFF' * (BOOTLOADER_SIZE - len(bootloader_data))

    # Combine bootloader and app data
    merged_data = bootloader_data
    if app_a_data:
        merged_data += app_a_data
    if app_b_data:
        merged_data += b'\xFF' * (APP_SIZE - len(app_a_data))
        merged_data += app_b_data

    with open(output_file, 'wb') as output_f:
        output_f.write(merged_data)

if __name__ == '__main__':
    # Example usage
    bootloader_file = 'bootloader.bin'
    app_file = 'app.bin'
    output_file = 'merged.bin'

    merge_bin_files(bootloader_file, app_file, output_file)
