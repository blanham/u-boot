// SPDX-License-Identifier: GPL-2.0+ OR MIT
/*
 * Copyright (c) 2023, Bryce Lanham
 */

#if !defined _EFI_SERIAL_H_
#define _EFI_SERIAL_H_

#include <efi.h>
#include <efi_api.h>

typedef enum {
    DefaultParity,
    NoParity,
    EvenParity,
    OddParity,
    MarkParity,
    SpaceParity
} efi_parity_type_t;

typedef enum {
    DefaultStopBits,
    OneStopBit,
    OneFiveStopBits,
    TwoStopBits
} efi_stop_bits_type_t;

typedef struct {
    u32 control_mask;
    u32 timeout;
    u64 baud_rate;
    u32 receive_fifo_depth;
    u32 data_bits;
    efi_parity_type_t parity;
    efi_stop_bits_type_t stop_bits;
} efi_serial_io_mode_t;

#define EFI_SERIAL_CLEAR_TO_SEND                   0x0010  // RO
#define EFI_SERIAL_DATA_SET_READY                  0x0020  // RO
#define EFI_SERIAL_RING_INDICATE                   0x0040  // RO
#define EFI_SERIAL_CARRIER_DETECT                  0x0080  // RO
#define EFI_SERIAL_REQUEST_TO_SEND                 0x0002  // WO
#define EFI_SERIAL_DATA_TERMINAL_READY             0x0001  // WO
#define EFI_SERIAL_INPUT_BUFFER_EMPTY              0x0100  // RO
#define EFI_SERIAL_OUTPUT_BUFFER_EMPTY             0x0200  // RO
#define EFI_SERIAL_HARDWARE_LOOPBACK_ENABLE        0x1000  // RW
#define EFI_SERIAL_SOFTWARE_LOOPBACK_ENABLE        0x2000  // RW
#define EFI_SERIAL_HARDWARE_FLOW_CONTROL_ENABLE    0x4000  // RW

struct efi_serial_io_protocol {
    u32 revision;
    efi_status_t (EFIAPI *reset)(struct efi_serial_io_protocol *protocol);
    efi_status_t (EFIAPI *set_attributes)(struct efi_serial_io_protocol *protocol, u64 baud_rate, u32 receive_fifo_depth, u32 timeout, efi_parity_type_t parity, u8 data_bits, efi_stop_bits_type_t stop_bits);
    efi_status_t (EFIAPI *set_control_bits)(struct efi_serial_io_protocol *protocol, u32 control);
    efi_status_t (EFIAPI *get_control_bits)(struct efi_serial_io_protocol *protocol, u32 *control);
    efi_status_t (EFIAPI *write)(struct efi_serial_io_protocol *protocol, u64 *buffer_size, void *buffer);
    efi_status_t (EFIAPI *read)(struct efi_serial_io_protocol *protocol, u64 *buffer_size, void *buffer);

    efi_serial_io_mode_t *mode;
};

efi_status_t platform_get_rng_device(struct udevice **dev);

#endif /* _EFI_SERIAL_H_ */

