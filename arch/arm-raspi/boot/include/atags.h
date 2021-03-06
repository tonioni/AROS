/*
    Copyright � 2013, The AROS Development Team. All rights reserved.
    $Id$
*/

#ifndef ATAGS_H_
#define ATAGS_H_

#include <inttypes.h>

typedef uint16_t uint16_t;
typedef uint8_t uint8_t;

#define COMMAND_LINE_SIZE 1024

/* The list ends with an ATAG_NONE node. */
#define ATAG_NONE       0x00000000

struct tag_header {
	uint32_t	size;
	uint32_t	tag;
};

/* The list must start with an ATAG_CORE node */
#define ATAG_CORE       0x54410001

struct tag_core {
	uint32_t	flags;            /* bit 0 = read-only */
	uint32_t	pagesize;
	uint32_t	rootdev;
};

/* it is allowed to have multiple ATAG_MEM nodes */
#define ATAG_MEM        0x54410002

struct tag_mem32 {
	uint32_t	size;
	uint32_t	start;  /* physical start address */
};

/* VGA text type displays */
#define ATAG_VIDEOTEXT  0x54410003

struct tag_videotext {
	uint8_t            x;
	uint8_t            y;
	uint16_t           video_page;
	uint8_t            video_mode;
	uint8_t            video_cols;
	uint16_t           video_ega_bx;
	uint8_t            video_lines;
	uint8_t            video_isvga;
	uint16_t           video_points;
};

/* describes how the ramdisk will be used in kernel */
#define ATAG_RAMDISK    0x54410004

struct tag_ramdisk {
	uint32_t flags;    /* bit 0 = load, bit 1 = prompt */
	uint32_t size;     /* decompressed ramdisk size in _kilo_ bytes */
	uint32_t start;    /* starting block of floppy-based RAM disk image */
};

/* describes where the compressed ramdisk image lives (virtual address) */
/*
 * this one accidentally used virtual addresses - as such,
 * it's deprecated.
 */
#define ATAG_INITRD     0x54410005

/* describes where the compressed ramdisk image lives (physical address) */
#define ATAG_INITRD2    0x54420005

struct tag_initrd {
	uint32_t start;    /* physical start address */
	uint32_t size;     /* size of compressed ramdisk image in bytes */
};

/* board serial number. "64 bits should be enough for everybody" */
#define ATAG_SERIAL     0x54410006

struct tag_serialnr {
	uint32_t low;
	uint32_t high;
};

/* board revision */
#define ATAG_REVISION   0x54410007

struct tag_revision {
	uint32_t rev;
};

/* initial values for vesafb-type framebuffers. see struct screen_info
 * in include/linux/tty.h
 */
#define ATAG_VIDEOLFB   0x54410008

struct tag_videolfb {
	uint16_t           lfb_width;
	uint16_t           lfb_height;
	uint16_t           lfb_depth;
	uint16_t           lfb_linelength;
	uint32_t           lfb_base;
	uint32_t           lfb_size;
	uint8_t            red_size;
	uint8_t            red_pos;
	uint8_t            green_size;
	uint8_t            green_pos;
	uint8_t            blue_size;
	uint8_t            blue_pos;
	uint8_t            rsvd_size;
	uint8_t            rsvd_pos;
};

/* command line: \0 terminated string */
#define ATAG_CMDLINE    0x54410009

struct tag_cmdline {
	char    cmdline[1];     /* this is the minimum size */
};

/* acorn RiscPC specific information */
#define ATAG_ACORN      0x41000101

struct tag_acorn {
	uint32_t memc_control_reg;
	uint32_t vram_pages;
	uint8_t sounddefault;
	uint8_t adfsdrives;
};

/* footbridge memory clock, see arch/arm/mach-footbridge/arch.c */
#define ATAG_MEMCLK     0x41000402

struct tag_memclk {
	uint32_t fmemclk;
};

struct tag {
	struct tag_header hdr;
	union {
		struct tag_core         core;
		struct tag_mem32        mem;
		struct tag_videotext    videotext;
		struct tag_ramdisk      ramdisk;
		struct tag_initrd       initrd;
		struct tag_serialnr     serialnr;
		struct tag_revision     revision;
		struct tag_videolfb     videolfb;
		struct tag_cmdline      cmdline;

		/*
		 * Acorn specific
		 */
		 struct tag_acorn        acorn;

		/*
		 * DC21285 specific
		 */
		 struct tag_memclk       memclk;
	} u;
};

#define tag_member_present(tag,member)                          \
		((unsigned long)(&((struct tag *)0L)->member + 1)       \
				<= AROS_LE2LONG((tag)->hdr.size * 4))

#define tag_next(t)     ((struct tag *)((uint32_t *)(t) + AROS_LE2LONG((t)->hdr.size)))
#define tag_size(type)  ((sizeof(struct tag_header) + sizeof(struct type)) >> 2)

#define for_each_tag(t,base)            \
		for (t = base; t->hdr.size; t = tag_next(t))


#endif /* ATAGS_H_ */
