// SPDX-License-Identifier: GPL-2.0+ OR Apache-2.0
/*
 *  EFI application Serial IO support
 *
 *  Copyright (C) 2023, Bryce Lanham <blanham@gmail.com>
 */

#define LOG_CATEGORY LOGC_EFI

#include <common.h>
#include <dm.h>
#include <charset.h>
#include <efi_loader.h>
#include <efi_serial.h>
#include <log.h>
#include <malloc.h>
#include <serial.h>
#include <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

const efi_guid_t efi_guid_serial_io_protocol = EFI_SERIAL_IO_PROTOCOL_GUID;

/**
 * platform_get_serial_device() - retrieve serial device
 *
 * This function retrieves the serial device.
 *
 * This function may be overridden if special initialization is needed.
 *
 * @dev:	serial device
 * Return:	status code
 */
__weak efi_status_t platform_get_serial_device(struct udevice **dev)
{
	int ret;
	struct udevice *devp;

	ret = uclass_get_device(UCLASS_SERIAL, 0, &devp);
	if (ret) {
		debug("Unable to get serial device\n");
		return EFI_DEVICE_ERROR;
	}

	*dev = devp;

	return EFI_SUCCESS;
}

/**
 * efi_serial_io_reset() - reset the serial device
 *
 * Reset the serial device.
 *
 * @this:	pointer to the EFI_SERIAL_IO_PROTOCOL
 *
 * Return:	status code
 */
static efi_status_t EFIAPI efi_serial_io_reset(struct efi_serial_io_protocol *this)
{
	// Reset the serial device
	// Or, for uboot, just clear the FIFO
	int ret;
	struct udevice *devp;

	ret = platform_get_serial_device(&devp);
	if (ret)
		return ret;

	if (serial_get_ops(devp)->clear) {
		int r = serial_get_ops(devp)->clear(devp);

		if (r) {
			log_err("Unable to clear serial device\n");
			return EFI_DEVICE_ERROR;
		}

		return EFI_SUCCESS;
	}

	// return EFI_SUCCESS since a device without a clear function is still valid
	return EFI_SUCCESS;
}

/**
 * efi_serial_io_set_attributes() - set the serial device attributes
 *
 * Set the serial device attributes.
 *
 * @this:	pointer to the EFI_SERIAL_IO_PROTOCOL
 * @baud_rate:	baud rate
 * @receive_fifodepth:	receive FIFO depth
 * @timeout:	timeout
 * @parity:	parity
 * @data_bits:	data bits
 * @stop_bits:	stop bits
 *
 * Return:	status code
 */
static efi_status_t EFIAPI efi_serial_io_set_attributes(struct efi_serial_io_protocol *this, u64 baud_rate, u32 receive_fifo_depth, u32 timeout, efi_parity_type_t parity, u8 data_bits, efi_stop_bits_type_t stop_bits)
{
	int ret;
	struct udevice *devp;

	ret = platform_get_serial_device(&devp);
	if (ret)
		return ret;

	// TODO: how do we handle errors here? i.e. when the device doesn't support the requested baud rate
	ret = serial_get_ops(devp)->setbrg(devp, baud_rate);
	if (ret) {
		log_err("Unable to set serial device baud rate\n");
		return EFI_DEVICE_ERROR;
	}

	int u_parity;
	switch (parity) {
		case DefaultParity:
		case NoParity:
			u_parity = SERIAL_PAR_NONE;
			break;
		case EvenParity:
			u_parity = SERIAL_PAR_EVEN;
			break;
		case OddParity:
			u_parity = SERIAL_PAR_ODD;
			break;
		case MarkParity:
		case SpaceParity:
		default:
			log_err("Invalid parity value %d\n", parity);
			return EFI_INVALID_PARAMETER;
	}

	int u_stop_bits;
	switch (stop_bits) {
		case DefaultStopBits:
			u_stop_bits = SERIAL_ONE_STOP;
			break;
		case OneStopBit:
			u_stop_bits = SERIAL_ONE_STOP;
			break;
		case OneFiveStopBits:
			u_stop_bits = SERIAL_ONE_HALF_STOP;
			break;
		case TwoStopBits:
			u_stop_bits = SERIAL_TWO_STOP;
			break;
		default:
			log_err("Invalid stop bits value %d\n", stop_bits);
			return EFI_INVALID_PARAMETER;
	}

	int u_data_bits;
	switch (data_bits) {
		case 5:
			u_data_bits = SERIAL_5_BITS;
			break;
		case 6:
			u_data_bits = SERIAL_6_BITS;
			break;
		case 7:
			u_data_bits = SERIAL_7_BITS;
			break;
		case 8:
			u_data_bits = SERIAL_8_BITS;
			break;
		default:
			log_err("Invalid data bits value %d\n", data_bits);
			return EFI_INVALID_PARAMETER;
	}

	uint serial_config = SERIAL_CONFIG(u_parity, u_data_bits, u_stop_bits);

	ret = serial_get_ops(devp)->setconfig(devp, serial_config);
	if (ret) {
		log_err("Unable to set serial device config\n");
		return EFI_DEVICE_ERROR;
	}

	return EFI_SUCCESS;
}

/**
 * efi_serial_io_set_control_bits() - set the serial device control bits
 *
 * Set the serial device control bits.
 *
 * @this:	pointer to the EFI_SERIAL_IO_PROTOCOL
 * @control:	control bits
 *
 * Return:	status code
 */
static efi_status_t EFIAPI efi_serial_io_set_control_bits(struct efi_serial_io_protocol *this, u32 control)
{
	// Set the serial device control bits
	// XXX: I don't think the UBOOT serial API supports this
	return EFI_UNSUPPORTED;
}

/**
 * efi_serial_io_get_control_bits() - get the serial device control bits
 *
 * Get the serial device control bits.
 *
 * @this:	pointer to the EFI_SERIAL_IO_PROTOCOL
 * @control:	pointer to control bits
 *
 * Return:	status code
 */
static efi_status_t EFIAPI efi_serial_io_get_control_bits(struct efi_serial_io_protocol *this, u32 *control)
{
	// Get the serial device control bits
	// XXX: I don't think the UBOOT serial API supports this
	return EFI_UNSUPPORTED;
}

/**
 * efi_serial_io_write() - write to the serial device
 *
 * Write to the serial device.
 *
 * @this:	pointer to the EFI_SERIAL_IO_PROTOCOL
 * @buffer_size:	buffer size
 * @buffer:	pointer to buffer
 *
 * Return:	status code
 */
static efi_status_t EFIAPI efi_serial_io_write(struct efi_serial_io_protocol *this, u64 *buffer_size, void *buffer)
{
	int ret;
	struct udevice *devp;

	if (!buffer_size || !*buffer_size || !buffer)
		return EFI_INVALID_PARAMETER;

	ret = platform_get_serial_device(&devp);
	if (ret)
		return EFI_DEVICE_ERROR;
	
	ret = serial_get_ops(devp)->putc(devp, *(char *)buffer);
	*buffer_size = 1;
	// XXX: This didn't work for me, but I'm not sure why
	//ret = serial_get_ops(devp)->puts(devp, buffer, *buffer_size);
	//if (ret) {
	//	log_err("Unable to write to serial device\n");
	//	return EFI_DEVICE_ERROR;
	//}

	return EFI_SUCCESS;
}

/**
 * efi_serial_io_read() - read from the serial device
 *
 * Read from the serial device.
 *
 * @this:	pointer to the EFI_SERIAL_IO_PROTOCOL
 * @buffer_size:	buffer size
 * @buffer:	pointer to buffer
 *
 * Return:	status code
 */
static efi_status_t EFIAPI efi_serial_io_read(struct efi_serial_io_protocol *this, u64 *buffer_size, void *buffer)
{
	int ret;
	struct udevice *devp;

	if (!buffer_size || !*buffer_size || !buffer)
		return EFI_INVALID_PARAMETER;

	ret = platform_get_serial_device(&devp);
	if (ret)
		return EFI_DEVICE_ERROR;
	
	ret = serial_get_ops(devp)->getc(devp);
	if (ret == -EAGAIN) {
		*buffer_size = 0;
		return EFI_TIMEOUT;
	} else if (ret < 0 || ret > 0xff) {
		log_err("Unable to read from serial device\n");
		return EFI_DEVICE_ERROR;
	}

	uint8_t *buf = buffer;

	*buf = (uint8_t )(ret & 0xFF);
	*buffer_size = 1;

	return EFI_SUCCESS;
}

static efi_serial_io_mode_t efi_serial_io_mode = {
	.control_mask = 0x3f,
	.timeout = 0,
	.baud_rate = 115200,
	.receive_fifo_depth = 1,
	.data_bits = 8,
	.parity = NoParity,
	.stop_bits = OneStopBit,
};

static struct efi_serial_io_protocol efi_serial_protocol = {
	.revision = 1,
	.reset = efi_serial_io_reset,
	.set_attributes = efi_serial_io_set_attributes,
	.set_control_bits = efi_serial_io_set_control_bits,
	.get_control_bits = efi_serial_io_get_control_bits,
	.write = efi_serial_io_write,
	.read = efi_serial_io_read,
	.mode = &efi_serial_io_mode,
};

/**
 * efi_serial_register() - register EFI_SERIAL_IO_PROTOCOL
 *
 * If a serial device is found, register it with the EFI_SERIAL_IO_PROTOCOL.
 *
 * Return:	status code
 */
efi_status_t efi_serial_register(void)
{
	efi_status_t ret;
	struct udevice *dev;

	ret = platform_get_serial_device(&dev);
	if (ret != EFI_SUCCESS) {
		log_warning("No Serial device found for EFI_SERIAL_IO_PROTOCOL\n");
		return EFI_SUCCESS;
	}

	ret = efi_add_protocol(efi_root, &efi_guid_serial_io_protocol, (void *)&efi_serial_protocol);
	if (ret != EFI_SUCCESS)
		log_err("Failed to add EFI_SERIAL_IO_PROTOCOL\n");

	log_crit("Registered EFI_SERIAL_IO_PROTOCOL\n");

	return ret;
}