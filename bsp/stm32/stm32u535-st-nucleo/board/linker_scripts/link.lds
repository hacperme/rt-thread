/*
 * linker script for STM32L4XX with GNU ld
 */

BOOTLOADER_ROM_SIZE = 512K;
APP_ROM_SIZE = 0K;

BOOTLOADER_SRAM_SIZE = 64K;
APP_SRAM_SIZE = 0K;

HEAP_RAM_SIZE = 192K;

/* Program Entry, set to mark it as "used" and avoid gc */
MEMORY
{
    ROM (rx) : ORIGIN = 0x08000000, LENGTH = BOOTLOADER_ROM_SIZE      /* 512KB flash */
    SRAM (rw) : ORIGIN = 0x20000000, LENGTH = BOOTLOADER_SRAM_SIZE    /* 64KB sram */
}
ENTRY(Reset_Handler)
_system_stack_size = 0x400;

SECTIONS
{
    ASSERT(BOOTLOADER_ROM_SIZE + APP_ROM_SIZE <= 512K, "The combined Flash partitions of the Bootloader and App exceed the total Flash size.")
    ASSERT(BOOTLOADER_SRAM_SIZE + APP_SRAM_SIZE + HEAP_RAM_SIZE <= 256K, "The combined RAM partitions of the Bootloader and App (excluding the heap area) exceed the total size of SRAM.")

    .text :
    {
        . = ALIGN(4);
        _stext = .;
        KEEP(*(.isr_vector))            /* Startup code */

        . = ALIGN(4);
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
    } > ROM = 0

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

    PROVIDE ( __bootloader_rom_start = ORIGIN(ROM) );
    PROVIDE ( __bootloader_rom_end = ORIGIN(ROM) + BOOTLOADER_ROM_SIZE );

    PROVIDE ( __bootloader_rom_occupied_end = LOADADDR(.data) + SIZEOF(.data) );

    ASSERT(__bootloader_rom_occupied_end  <= __bootloader_rom_end, "Bootloader ROM data exceeds bootloader ROM end.")

    PROVIDE ( __app_rom_start = __bootloader_rom_end );
    PROVIDE ( __app_rom_end = __app_rom_start + APP_ROM_SIZE);

    .stack :
    {
        . = ALIGN(4);
        _sstack = .;
        . = . + _system_stack_size;
        . = ALIGN(4);
        _estack = .;
    } > SRAM

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

        *(.bss.init)
    } > SRAM

    __bss_end = .;

    PROVIDE ( __bootloader_ram_start = ORIGIN(SRAM) );
    PROVIDE ( __bootloader_ram_end = ORIGIN(SRAM) + BOOTLOADER_SRAM_SIZE );

    PROVIDE ( __bootloader_ram_occupied_end = __bss_end );

    ASSERT(__bootloader_ram_occupied_end  <= __bootloader_ram_end, "Bootloader RAM data exceeds bootloader RAM end.")

    PROVIDE ( __app_ram_start = __bootloader_ram_end );
    PROVIDE ( __app_ram_end = __app_ram_start + APP_SRAM_SIZE);

    _end = __app_ram_end;

    PROVIDE ( __heap_start = _end );
    PROVIDE ( __heap_end = __heap_start + HEAP_RAM_SIZE);

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
