BOOTLOADER_SIZE = 328 * 1024
APP_SIZE = 184 * 1024

def merge_bin_files(bootloader_file, app_file, output_file):
    with open(bootloader_file, 'rb') as bootloader_f:
        bootloader_data = bootloader_f.read()

    with open(app_file, 'rb') as app_f:
        app_data = app_f.read()

    # Pad the bootloader data with 0xFF if its size is less than BOOTLOADER_SIZE
    if len(bootloader_data) < BOOTLOADER_SIZE:
        bootloader_data += b'\xFF' * (BOOTLOADER_SIZE - len(bootloader_data))

    # Combine bootloader and app data
    merged_data = bootloader_data + app_data

    with open(output_file, 'wb') as output_f:
        output_f.write(merged_data)

if __name__ == '__main__':
    # Example usage
    bootloader_file = 'bootloader.bin'
    app_file = 'app.bin'
    output_file = 'merged.bin'

    merge_bin_files(bootloader_file, app_file, output_file)
