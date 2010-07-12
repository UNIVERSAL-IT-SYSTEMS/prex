/*
 * Linker script for boot loader
 */
INCLUDE config.ld
OUTPUT_FORMAT("elf32-h8300", "elf32-h8300",
              "elf32-h8300")
OUTPUT_ARCH(h8300s)
ENTRY(_start)

SECTIONS {
	. = CONFIG_LOADER_TEXT;
	
	__cyboot_ram_size = 0xffec04;
	
	.text . :
	{
		*(.text)
		*(.text.*)
	}
	
	.rodata :
	{
		*(.rodata)
		*(.rodata.*)
		*(.rodata1)
		. = ALIGN(4);
		__data_in_rom_start = .;
	}
	
	. = CONFIG_RAM_START;
	.data : AT(__data_in_rom_start) {
		__data_in_ram_start = .;
		*(.data)
		*(.data.*)
		*(.data1)
		. = ALIGN(4);
		__data_in_ram_end = .;
	}
	
	. = ALIGN(4);
	.bss : {
		__bss_start = .;
		*(.bss)
		*(.bss.*)
		*(COMMON)
		. = ALIGN(4);
		__bss_end = .;
	}
	
	/DISCARD/ :
	{
		*(.comment)
		*(.shstrtab)
		*(.symtab)
		*(.strtab)
	}
}
