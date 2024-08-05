/*
 * linker script for STM32L4XX with GNU ld
 */

BOOTLOADER_ROM_SIZE = 256K;
APP_ROM_SIZE = 256K;

BOOTLOADER_SRAM1_SIZE = 160K;
APP_SRAM1_SIZE = 32K;

HEAP_RAM2_SIZE = 64K;

/* Program Entry, set to mark it as "used" and avoid gc */
MEMORY
{
    ROM (rx) : ORIGIN = 0x08000000, LENGTH = 512K     /* 512KB flash */
    SRAM1 (rw) : ORIGIN = 0x20000000, LENGTH =  192K  /* 192KB sram1 */
    SRAM2 (rw) : ORIGIN = 0x20030000, LENGTH =  64K   /* 64KB sram2 */
}
ENTRY(Reset_Handler)
_system_stack_size = 0x400;

SECTIONS
{
    ASSERT(BOOTLOADER_ROM_SIZE + APP_ROM_SIZE <= LENGTH(ROM), "The combined Flash partitions of the Bootloader and App exceed the total Flash size.")
    ASSERT(BOOTLOADER_SRAM1_SIZE + APP_SRAM1_SIZE <= LENGTH(SRAM1), "The combined RAM partitions of the Bootloader and App (excluding the heap area) exceed the total size of SRAM1.")

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
    } > SRAM1

    PROVIDE ( __bootloader_rom_start = ORIGIN(ROM) );
    PROVIDE ( __bootloader_rom_end = ORIGIN(ROM) + BOOTLOADER_ROM_SIZE );

    PROVIDE ( __bootloader_rom_occupied_end = LOADADDR(.data) + SIZEOF(.data) );

    ASSERT(__bootloader_rom_occupied_end  <= __bootloader_rom_end, "Bootloader ROM data exceeds bootloader ROM end.")

    PROVIDE ( __app_rom_start = __bootloader_rom_end );
    PROVIDE ( __app_rom_end = ORIGIN(ROM) + LENGTH(ROM));

    .stack :
    {
        . = ALIGN(4);
        _sstack = .;
        . = . + _system_stack_size;
        . = ALIGN(4);
        _estack = .;
    } > SRAM1

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
    } > SRAM1

    __bss_end = .;

    PROVIDE ( __bootloader_ram_start = ORIGIN(SRAM1) );
    PROVIDE ( __bootloader_ram_end = ORIGIN(SRAM1) + BOOTLOADER_SRAM1_SIZE );

    PROVIDE ( __bootloader_ram_occupied_end = __bss_end );

    ASSERT(__bootloader_ram_occupied_end  <= __bootloader_ram_end, "Bootloader RAM data exceeds bootloader RAM end.")

    PROVIDE ( __app_ram_start = __bootloader_ram_end );
    PROVIDE ( __app_ram_end = ORIGIN(SRAM1) + LENGTH(SRAM1));

    _end = ORIGIN(SRAM2);

    PROVIDE ( __heap_start = _end );
    PROVIDE ( __heap_end = __heap_start + LENGTH(SRAM2));

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
