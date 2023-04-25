/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2017 Tuomas Tynkkynen
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/* Physical memory map */
#define CFG_SYS_SDRAM_BASE		0x40000000

/* For timer, QEMU emulates an ARMv7/ARMv8 architected timer */


#ifdef CONFIG_VIDEO_RAMFB
# define QEMU_STDOUT "serial,vidconsole"
#else
# define QEMU_STDOUT "serial"
#endif

#include <config_distro_bootcmd.h>

#define CONFIG_EXTRA_ENV_SETTINGS \
        "pxefile_addr_r=0x40300000\0" \
        "kernel_addr_r=0x40400000\0" \
        "ramdisk_addr_r=0x44000000\0" \
       "stdin=serial\0" \
       "stdout=" QEMU_STDOUT "\0" \
       "stderr=" QEMU_STDOUT "\0" \
        BOOTENV

#endif /* __CONFIG_H */
