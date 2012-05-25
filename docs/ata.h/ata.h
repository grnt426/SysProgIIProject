ata.h - DERP OS man page

Name -------------------
ata.h - ATA Interface Library

Synopsis ---------------

#include "ata.h"

void _ata_probe(Uint16 bus, Uint16 device, Uint16 func);
void _ata_initialize(ATAController *cont, Uint16 bus, Uint16 device, Uint16 func);
Uint8 _ata_read_reg(ATAChannel channel, ATAReg reg);
void _ata_write_reg(ATAChannel channel, ATAReg reg, Uint8 payload);
Uint16 _ata_get_bar(Uint16 bus, Uint16 device, Uint16 func, Uint16 offset);
void _ata_read_sector(ATADevice *dev, Uint32 lba, ATASector *d);
void _ata_write_sector(ATADevice *dev, Uint32 lba, ATASector *s);
void _ata_wait( void );
Uint8 _ata_wait_bsy(ATAChannel channel);
void _ata_blank_sector(ATASector *s);
Uint32 _sector_get_long(ATASector *s, Uint16 offset);
Uint16 _sector_get_word(ATASector *s, Uint16 offset);
void _sector_put_long(ATASector *s, Uint16 offset, Uint32 payload);
void _sector_put_word(ATASector *s, Uint16 offset, Uint16 payload);

Description ------------

The ATA interface library provides functionality for interacting at a low-level with
the ATA devices of the system. The system currently only allows for interacting with
ATA devices only (not ATAPI) that are on a SATA controller. IDE devices are not
supported nor are any device that does not support the ATA-7 standard or higher.
All interaction for reading and writing sectors are performed using LBA-48
(and technically only using 32 bits worth of LBA).

Theoretically, there is no need for userspace functions to access the functions
in this library.

The module is broken into two main portions:

Initialization -

The initialization function _ata_probe(), _ata_initialize(), 
_ata_read/write_reg() and _ata_get_bar() are used to scan the PCI bus devices
that are mass storage class and find any/all ATA devices that are on that
controller. The devices are stored as ATADevice structs and globally allocated.

Reading/Writing -

Reading and writing occur utilizing polling IO with an ATADevices. This
basically means that data is read and written from the device when the device
declares that it is ready. A wait function has been constructed to wait until
the device is ready, or kicks out if there was an error on the drive.

There is very little error handling within this library, so if the device
faults during an IO event, the function will fail silently.

Dependencies -----------

pci.h

Functions --------------

_ata_probe() - Probe a mass storage controller for ATADevices
_ata_initialize() - Initialize a ATADevice
_ata_read_reg() - Read from a register on the ATA controller
_ata_write_reg() - Write to a register on the ATA controller
_ata_get_bar() - Get a base address register from the ATA controller
_ata_read_sector() - Read a sector from the hard drive
_ata_write_sector() - Write a sector to the hard drive
_ata_wait() - Wait for a count of 1000 for the drive sel command to take effect
_ata_wait_bsy() - Wait until the hard drive is ready or errors out
_ata_blank_sector() - Clear a sector in memory out with 0's
_sector_get_long() - Retrieve a little-endian long word from a sector in memory
_sector_get_word() - Retrieve a little-endian word from a sector in memory
_sector_put_long() - Store a long word in a sector in memory as little-endian
_sector_put_word() - Store a word in a sector in memory as little-endian

Constants --------------

ATA_PCI_REG_VENDOR - PCI register for vendor
ATA_PCI_REG_DEVICE - PCI register for device class
ATA_PCI_REG_HEADER - PCI register for header type
ATA_PCI_REG_PCMD - PCI register for primary command BAR
ATA_PCI_REG_PCTRL - PCI register for primary control BAR
ATA_PCI_REG_SCMD - PCI register for secondary command BAR
ATA_PCI_REG_SCTRL - PCI register for secondary control BAR
ATA_PCI_REG_BMAST - PCI register for bus master BAR
ATA_PCI_REG_ABAR - PCI register for ABAR (busmaster of AHCI)
ATA_PCI_REG_MAP - PCI register for MAP (sets mode of controller to IDE/SATA)

ATA_PORT_EMPTY - Port is not in use
ATA_PORT_INUSE - Port is in use
ATA_PORT_CHANPRI - Device is on primary channel of controller
ATA_PORT_CHANSEC - Device is on secondary channel of controller
ATA_PORT_CHANMAST - Device is the master of the channel
ATA_PORT_CHANSLAV - Device is slave of the channel

ATA_TYPE_ATA - Device is ATA
ATA_TYPE_ATAPI - Device is ATAPI

ATA_IDENT_MODEL - Offset into IDENTIFY command data block for model string

ATA_STATUS_BUSY - Mask for device status register busy bit
ATA_STATUS_DRQ - Mask for device status register disk request bit
ATA_STATUS_RDY - Mask for device status register device ready bit
ATA_STATUS_ERR - Mask for device status register error bit
ATA_NOINT - Mask for device status register no-interrupt bit

ATA_STATUS_OK - Mask for device status register to say device is OK

ATA_DEFAULT_PCMD - Default IO port for primary command
ATA_DEFAULT_PCTRL - Default IO port for primary control
ATA_DEFAULT_SCMD - Default IO port for secondary command
ATA_DEFAULT_SCTRL - Default IO port for secondary control
ATA_DEFAULT_BMAST - Default IO port for busmaster

Data Structures --------

The following structs are defined with typedefs

ATAChannel struct
	Uint32 command - The command register (BAR)
	Uint32 control - The control register (should be BAR + 8)
	Uint32 busmast - The bus master register (PRI = BAR4, SEC = BAR4 + 8);
	
ATADevice struct
	ATAChannel channel - The channel the device is connected to
	Uint32 size - The size in sectors of the device
	Uint8 device - 0 for primary, 1 for secondary
	Uint8 type - 0 for ATA, 1 for ATAPI
	char serial[21] - The serial number of the drive (NULL terminated)
	char model[41] - The model number of the drive (NULL terminated)

typedef Uint8 ATASector[512] - A representation of a sector as 512 bytes

typedef ATAChannel ATAController[2] - A controller consists of two channels

Global Variables -------

ATADevice ata_devices[10] - All the devices discovered on boot
Uint8 ata_device_count - The number of devices discovered on boot

Enums ------------------

ATAReg
	ATA_REG_DATA - The data register
	ATA_REG_ERROR - On read, error register.
	ATA_REG_FEATURES - On write, features reg
	ATA_REG_SECCOUNT1 - Sectors to read/write
	ATA_REG_LBA0 - LBA 0:7
	ATA_REG_LBA1 - LBA 8:15
	ATA_REG_LBA2 - LBA 16:23
	ATA_REG_DRIVESEL - Drive selector register
	ATA_REG_STATUS - On read, status register.
	ATA_REG_COMMAND	- On write, command reg
	ATA_REG_SECCOUNT2 - High byte of number of sectors
	ATA_REG_LBA3 - LBA 24:31
	ATA_REG_LBA4 - LBA 32:39
	ATA_REG_LBA5 - LBA 40:47
	ATA_REG_ALTSTATUS -  On read, alternate status register
	ATA_REG_CONTROL	- On write, control reg
	ATA_REG_DEVADDR	- Device address?
	
ATACommand
	ATA_CMD_IDENTIFY - Tell the device to identify itself
	ATA_CMD_READSECE - Read sectors using 48-bit LBA
	ATA_CMD_WRITSECE - Write sectors using 48-bit LBA 
	ATA_CMD_FLUSHE - Flush the 48bit LBA cache