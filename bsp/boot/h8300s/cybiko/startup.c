/*-
 * Copyright (c) 2008-2009, Kohsuke Ohtani
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the author nor the names of any co-contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#define KERNEL
#include <sys/param.h>
#include <sys/bootinfo.h>
#include <sys/types.h>
#include <boot.h>
#include <config.h>

extern char _bss_end;
extern char binary_bootdisk_crom_start;
extern char binary_bootdisk_crom_size;

extern void driver_main(dkifn_t *dkient);

extern void _start_boot(void);
extern void _start_proc(void);
extern void _start_exec(void);
extern void _start_fs(void);

extern int _cyboot_ram_size;

static const struct module startup_tasks[] =
{
	{
		.name = "boot",
		.entry = (vaddr_t) &_start_boot,
		.textsz = (size_t) &_bss_end
	},
	{
		.name = "proc",
		.entry = (vaddr_t) &_start_proc,
		.textsz = (size_t) &_bss_end
	},
	{
		.name = "exec",
		.entry = (vaddr_t) &_start_exec,
		.textsz = (size_t) &_bss_end
	},
	{
		.name = "fs",
		.entry = (vaddr_t) &_start_fs,
		.textsz = (size_t) &_bss_end
	}
};

#define NR_TASKS (sizeof(startup_tasks) / sizeof(*startup_tasks))

static char bootinfo_bytes[sizeof(struct bootinfo) +
                            sizeof(struct module) * (NR_TASKS-1)];
struct bootinfo* const bootinfo = (struct bootinfo*) bootinfo_bytes;

/*
 * Setup boot information.
 */
static void
bootinfo_init(void)
{
	struct bootinfo *bi = bootinfo;

	/* Screen size. This is set up for the HD66421 driver. */

	bi->video.text_x = 160 / 8;
	bi->video.text_y = 100 / 8;

	/* On-board RAM */

	int ramstart = (int) &_bss_end;
	ramstart = (ramstart + PAGE_SIZE-1) & ~(PAGE_SIZE-1);

	bi->ram[0].base = ramstart;
	bi->ram[0].size = (CONFIG_RAM_START + _cyboot_ram_size) - ramstart;
	bi->ram[0].type = MT_USABLE;

#if defined CONFIG_CROMDISK || defined CONFIG_RAMDISK
	bi->bootdisk.base = (paddr_t) &binary_bootdisk_crom_start;
	bi->bootdisk.size = (paddr_t) &binary_bootdisk_crom_size;
	bi->bootdisk.type = MT_BOOTDISK;
#endif

	bi->nr_rams = 1;
	bi->driver.entry = (int) driver_main;

	bi->nr_tasks = NR_TASKS;
	for (int i = 0; i < NR_TASKS; i++)
		bi->tasks[i] = startup_tasks[i];
}

void
startup(void)
{
	bootinfo_init();
}
