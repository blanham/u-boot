// SPDX-License-Identifier: GPL-2.0+
/*
 * efi_selftest_serial
 *
 * Copyright (c) 2023 Bryce Lanham <blanham@gmail.com>
 *
 * Test the serial io protocol.
 */

#include <efi_selftest.h>
#include <efi_serial.h>


static struct efi_boot_services *boottime;
static efi_guid_t efi_serial_guid = EFI_SERIAL_IO_PROTOCOL_GUID;

/*
 * Setup unit test.
 *
 * @handle:	handle of the loaded image
 * @systable:	system table
 * Return:	EFI_ST_SUCCESS for success
 */
static int setup(const efi_handle_t handle,
		 const struct efi_system_table *systable)
{
	boottime = systable->boottime;
	return EFI_ST_SUCCESS;
}

/*
 * Execute unit test.
 *
 * Retrieve available SERIAL algorithms.
 *
 * Return:	EFI_ST_SUCCESS for success
 */
static int execute(void)
{
	efi_status_t ret;
	struct efi_serial_io_protocol *serial;

	/* Get random number generator protocol */
	ret = boottime->locate_protocol(&efi_serial_guid, NULL, (void **)&serial);
	if (ret != EFI_SUCCESS) {
		efi_st_error("Random number generator protocol not available\n");
		return EFI_ST_FAILURE;
	}

	// TODO: Write tests

	return EFI_ST_SUCCESS;
}

EFI_UNIT_TEST(serial) = {
	.name = "serial io",
	.phase = EFI_EXECUTE_BEFORE_BOOTTIME_EXIT,
	.setup = setup,
	.execute = execute,
};

