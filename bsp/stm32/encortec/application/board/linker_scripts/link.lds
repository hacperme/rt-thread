/*
 * linker script for STM32L4XX with GNU ld
 */
FLASH_ORIGIN = 0x08000000;
FLASH_BANK1_SIZE = 1024K;
FLASH_BANK2_SIZE = 1024K;

SRAM1_ORIGIN = 0x20000000;
SRAM1_SIZE = 192K;
SRAM2_ORIGIN = 0x20030000;
SRAM2_SIZE = 64K;
SRAM3_ORIGIN = 0x20040000;
SRAM3_SIZE = 512K;

SRAM4_ORIGIN = 0x28000000;
SRAM4_SIZE = 16K;

BACK_SRAM_ORIGIN = 0x40036400;
BACK_SRAM_SIZE = 2K;

FLASH_ROM_SIZE = FLASH_BANK1_SIZE + FLASH_BANK2_SIZE;
FLASH_RAM_SIZE = SRAM1_SIZE + SRAM2_SIZE + SRAM3_SIZE;

BOOTLOADER_ROM_SIZE = 512K;
MBR_ROM_SIZE = 16K;
APP_ROM_SIZE = 760K;

BOOTLOADER_RAM_SIZE = 64K;
APP_RAM_SIZE = 64K;

HEAP_RAM_SIZE = 640K;

/* Program Entry, set to mark it as "used" and avoid gc */
MEMORY
{
    ROM (rx) : ORIGIN = FLASH_ORIGIN + BOOTLOADER_ROM_SIZE + MBR_ROM_SIZE, LENGTH = APP_ROM_SIZE
    SRAM (rw) : ORIGIN = SRAM1_ORIGIN + BOOTLOADER_RAM_SIZE, LENGTH = APP_RAM_SIZE
}

ENTRY(main)

SECTIONS
{
    ASSERT(BOOTLOADER_ROM_SIZE + MBR_ROM_SIZE + APP_ROM_SIZE <= FLASH_ROM_SIZE - APP_ROM_SIZE, "The combined Flash partitions of the Bootloader and App exceed the total Flash size.")
    ASSERT(BOOTLOADER_RAM_SIZE + APP_RAM_SIZE + HEAP_RAM_SIZE <= FLASH_RAM_SIZE, "The combined RAM partitions of the Bootloader, App and heap exceed the total RAM size.")

    .text : AT(ORIGIN(ROM))
    {
        . = ALIGN(4);
        _stext = .;

	    PROVIDE_HIDDEN (__app_header_start = .);
		KEEP(*(.app_header))
		PROVIDE_HIDDEN (__app_header_end = .);

        *(.text)                        /* remaining code */
        *(.text.*)                      /* remaining code */
        *(.rodata)                      /* read-only data (constants) */
        *(.rodata*)
        *(.glue_7)
        *(.glue_7t)
        *(.gnu.linkonce.t*)

        /* section information for finsh shell */
        . = ALIGN(4);
        __fsymtab_start = .;
        KEEP(*(FSymTab))
        __fsymtab_end = .;

        . = ALIGN(4);
        __vsymtab_start = .;
        KEEP(*(VSymTab))
        __vsymtab_end = .;

        /* section information for initial. */
        . = ALIGN(4);
        __rt_init_start = .;
        KEEP(*(SORT(.rti_fn*)))
        __rt_init_end = .;

        . = ALIGN(4);

        PROVIDE(__ctors_start__ = .);
        KEEP (*(SORT(.init_array.*)))
        KEEP (*(.init_array))
        PROVIDE(__ctors_end__ = .);

        . = ALIGN(4);

        _etext = .;
    } > ROM

    /* .ARM.exidx is sorted, so has to go in its own output section.  */
    __exidx_start = .;
    .ARM.exidx :
    {
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)

        /* This is used by the startup in order to initialize the .data secion */
        _sidata = .;
    } > ROM
    __exidx_end = .;

    /* .data section which is used for initialized data */

    .data : AT (_sidata)
    {
        . = ALIGN(4);
        /* This is used by the startup in order to initialize the .data secion */
        _sdata = . ;

        *(.data)
        *(.data.*)
        *(.gnu.linkonce.d*)

        PROVIDE(__dtors_start__ = .);
        KEEP(*(SORT(.dtors.*)))
        KEEP(*(.dtors))
        PROVIDE(__dtors_end__ = .);

        . = ALIGN(4);
        /* This is used by the startup in order to initialize the .data secion */
        _edata = . ;
    } > SRAM

    PROVIDE ( __app_data_load_start = LOADADDR(.data) );
    PROVIDE ( __app_data_load_end = LOADADDR(.data) + SIZEOF(.data) );

    PROVIDE ( __app_rom_start = ORIGIN(ROM) );
    PROVIDE ( __app_rom_end = ORIGIN(ROM) + LENGTH(ROM) );

    PROVIDE ( __app_rom_occupied_end = __app_data_load_end );

    ASSERT(__app_rom_occupied_end  <= __app_rom_end, "Application ROM data exceeds application ROM end.")

    __bss_start = .;
    .bss :
    {
        . = ALIGN(4);
        /* This is used by the startup in order to initialize the .bss secion */
        _sbss = .;

        *(.bss)
        *(.bss.*)
        *(COMMON)

        . = ALIGN(4);
        /* This is used by the startup in order to initialize the .bss secion */
        _ebss = . ;
    } > SRAM

    __bss_end = .;

    PROVIDE ( __app_ram_start = ORIGIN(SRAM) );
    PROVIDE ( __app_ram_end = ORIGIN(SRAM) + LENGTH(SRAM) );

    PROVIDE ( __app_ram_occupied_end = __bss_end );

    ASSERT(__app_ram_occupied_end  <= __app_ram_end, "Application RAM data exceeds application RAM end.")

    /* Stabs debugging sections.  */
    .stab          0 : { *(.stab) }
    .stabstr       0 : { *(.stabstr) }
    .stab.excl     0 : { *(.stab.excl) }
    .stab.exclstr  0 : { *(.stab.exclstr) }
    .stab.index    0 : { *(.stab.index) }
    .stab.indexstr 0 : { *(.stab.indexstr) }
    .comment       0 : { *(.comment) }
    /* DWARF debug sections.
     * Symbols in the DWARF debugging sections are relative to the beginning
     * of the section so we begin them at 0.  */
    /* DWARF 1 */
    .debug          0 : { *(.debug) }
    .line           0 : { *(.line) }
    /* GNU DWARF 1 extensions */
    .debug_srcinfo  0 : { *(.debug_srcinfo) }
    .debug_sfnames  0 : { *(.debug_sfnames) }
    /* DWARF 1.1 and DWARF 2 */
    .debug_aranges  0 : { *(.debug_aranges) }
    .debug_pubnames 0 : { *(.debug_pubnames) }
    /* DWARF 2 */
    .debug_info     0 : { *(.debug_info .gnu.linkonce.wi.*) }
    .debug_abbrev   0 : { *(.debug_abbrev) }
    .debug_line     0 : { *(.debug_line) }
    .debug_frame    0 : { *(.debug_frame) }
    .debug_str      0 : { *(.debug_str) }
    .debug_loc      0 : { *(.debug_loc) }
    .debug_macinfo  0 : { *(.debug_macinfo) }
    /* SGI/MIPS DWARF 2 extensions */
    .debug_weaknames 0 : { *(.debug_weaknames) }
    .debug_funcnames 0 : { *(.debug_funcnames) }
    .debug_typenames 0 : { *(.debug_typenames) }
    .debug_varnames  0 : { *(.debug_varnames) }
}
