/*-
 * Copyright (c) 2010, David Given
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

/*
 * cromdisk.c - compressed ROM disk driver
 */

#include <driver.h>
#include "puff.h"

/* #define DEBUG_CROMDISK 1 */

#ifdef DEBUG_CROMDISK
#define DPRINTF(a)	printf a
#else
#define DPRINTF(a)
#endif

/* Block size */
#define BSIZE		512

struct cromdisk_softc {
	device_t dev;		/* device object */
	uint8_t* addr;      /* base address of image */
	size_t size;		/* uncompressed image size */
};

static int cromdisk_read(device_t, char *, size_t *, int);
static int cromdisk_probe(struct driver *);
static int cromdisk_init(struct driver *);

static struct devops cromdisk_devops = {
	/* open */	no_open,
	/* close */	no_close,
	/* read */	cromdisk_read,
	/* write */	no_write,
	/* ioctl */	no_ioctl,
	/* devctl */ no_devctl,
};

struct driver cromdisk_driver = {
	/* name */	"cromdisk",
	/* devops */ &cromdisk_devops,
	/* devsz */	sizeof(struct cromdisk_softc),
	/* flags */	0,
	/* probe */	cromdisk_probe,
	/* init */	cromdisk_init,
	/* shutdown */	NULL,
};

static int
cromdisk_read(device_t dev, char *buf, size_t *nbyte, int blkno)
{
	struct cromdisk_softc *sc = device_private(dev);
	int offset = blkno * BSIZE;
	uint8_t* kbuf;
	size_t nr_read;

	DPRINTF(("cromdisk_read: buf=%x nbyte=%d blkno=%x\n",
		 buf, *nbyte, blkno));

	/* Check overrun */
	if (offset > (int)sc->size) {
		DPRINTF(("cromdisk_read: overrun!\n"));
		return EIO;
	}
	nr_read = *nbyte;
	if (offset + nr_read > (int)sc->size)
		nr_read = sc->size - offset;

	/* Translate buffer address to kernel address */
	if ((kbuf = kmem_map(buf, nr_read)) == NULL) {
		return EFAULT;
	}

	/* Copy data */
	*nbyte = nr_read;
	while (nr_read > 0)
	{
		int e;
		unsigned long destlen = nr_read;
		uint8_t* p = sc->addr + 2 + (blkno * 4);
		unsigned long blksize = *p++ * 2;
		uint32_t blkoffset = p[0] | (p[1] << 8) | (p[2] << 16);
		uint8_t* blkaddr = sc->addr + blkoffset;

		sched_lock();
		e = puff(kbuf, &destlen, blkaddr, &blksize);
		sched_unlock();

		nr_read -= BSIZE;
		kbuf += BSIZE;
		blkno++;
	}
	return 0;
}

static int
cromdisk_probe(struct driver *self)
{
	struct bootinfo *bi;
	struct physmem *phys;

	machine_bootinfo(&bi);
	phys = &bi->bootdisk;
	if (phys->size == 0) {
#ifdef DEBUG
		printf("cromdisk: no bootdisk found...\n");
#endif
		return ENXIO;
	}
	return 0;
}

static int
cromdisk_init(struct driver *self)
{
	struct cromdisk_softc *sc;
	struct bootinfo *bi;
	struct physmem *phys;
	uint8_t* data;
	uint16_t blocks;
	device_t dev;

	machine_bootinfo(&bi);
	phys = &bi->bootdisk;
	data = (uint8_t*) ptokv(phys->base);
	blocks = data[0] | data[1]<<8;

	dev = device_create(self, "ram0", D_BLK|D_PROT);

	sc = device_private(dev);
	sc->dev = dev;
	sc->addr = (uint8_t*) data;
	sc->size = (size_t) blocks * BSIZE;

#ifdef DEBUG
	printf("CROM disk at 0x%08x (%dK bytes compressed, %dK bytes real)\n",
	       (u_int)sc->addr, phys->size/1024,
	       sc->size/1024);
#endif
	return 0;
}
