////////////////////////////////////////////////////////////////////////////
// SysProgII OS (DERP OS) - SATA Controller Header
//
// @file	ata.h
// @descrip	Declares the functions for interfacing with the SATA bus.
// @author	Benjamin Russell
////////////////////////////////////////////////////////////////////////////

#ifndef	ATA_H
#define ATA_H

// #DEFINES ////////////////////////////////////////////////////////////////

// PCI Configuration Addressing
#define SATA_PCI_BUS        0
#define SATA_PCI_DEVICE     31
#define SATA_PCI_FUNCTION   2

// PCI Configuration Register Offsets
#define SATA_PCI_REG_VENDOR 0x00
#define SATA_PCI_REG_DEVICE 0x02
#define SATA_PCI_REG_HEADER 0x0E	// 16 bit
#define SATA_PCI_REG_PCMD	0x10
#define SATA_PCI_REG_PCTRL	0x14
#define SATA_PCI_REG_SCMD	0x18
#define SATA_PCI_REG_SCTRL	0x1C
#define SATA_PCI_REG_BMAST	0x20
#define SATA_PCI_REG_ABAR   0x24
#define SATA_PCI_REG_MAP	0x90	// 8 bit

// IDE Types
#define ATA_PORT_EMPTY		0x0
#define ATA_PORT_INUSE		0x1
#define ATA_PORT_CHANPRI	0x0
#define ATA_PORT_CHANSEC	0x1
#define ATA_PORT_CHANMAST	0x0
#define ATA_PORT_CHANSLAV	0x1

#define ATA_TYPE_ATA		0x0
#define ATA_TYPE_ATAPI		0x1

// Important offsets into the identify space
#define ATA_IDENT_MODEL		0x54

// Useful bitmasks for the status register
#define ATA_STATUS_BUSY		0x80
#define ATA_STATUS_DRQ		0x08
#define ATA_STATUS_RDY		0x40
#define ATA_NOINT			0x02

// Determine if the device is ATAPI
#define ATAPI_LBA1			0x14
#define ATAPI_LBA2			0xEB
#define ATAPI_ALT_LBA1		0x69
#define ATAPI_ALT_LBA2		0x96


// TYPEDEFS ////////////////////////////////////////////////////////////////
typedef struct {
	Uint32	command;		// The command register (BAR)
	Uint32	control;		// The control register (should be BAR + 8)
	Uint32	busmast;		// The bus master register (PRI = BAR4, SEC = BAR4 + 8);
} ATAChannel;
typedef ATAChannel ATAController[2];

typedef enum {
	// Registers on the command
	ATA_REG_DATA		= 0x00,	// The data register
	ATA_REG_ERROR 		= 0x01,	// On read, error register.
	ATA_REG_FEATURES 	= 0x01, // On write, features reg
	ATA_REG_SECCOUNT1	= 0x02,	// Number of sectors
	ATA_REG_LBA0		= 0x03,	// LBA 0:7
	ATA_REG_LBA1		= 0x04,	// LBA 8:15
	ATA_REG_LBA2		= 0x05,	// LBA 16:23
	ATA_REG_DRIVESEL	= 0x06,	// Drive selector register
	ATA_REG_STATUS		= 0x07,	// On read, status register.
	ATA_REG_COMMAND		= 0x07,	// On write, command reg
	
	// Registers that are upper 8 bits of 
	ATA_REG_SECCOUNT2	= 0x08,	// High byte of number of sectors
	ATA_REG_LBA3		= 0x09, // LBA 24:31
	ATA_REG_LBA4		= 0x0A, // LBA 32:39
	ATA_REG_LBA5		= 0x0B, // LBA 40:47

	// Registers on the Bus Master
	ATA_REG_ALTSTATUS	= 0x0C, // On read, alternate status register
	ATA_REG_CONTROL		= 0x0C, // On write, control reg
	ATA_REG_DEVADDR		= 0x0D	// Device address?
} ATAReg;

typedef enum {
	ATA_CMD_IDENTIFY	= 0xEC,	// Tell the device to identify itself
	ATA_CMD_READSECE	= 0x24,	// Read sectors using 48-bit LBA
	ATA_CMD_WRITSECE	= 0x34,	// Write sectors using 48-bit LBA 
	ATA_CMD_FLUSHE		= 0xEA	// Flush the 48bit LBA cache
} ATACommand;

typedef struct {
	// VERY simple ATA device
	ATAChannel	channel;		// The channel the device is connected to
	Uint32		size;			// The size in sectors of the device
	Uint8		device;			// 0 for primary, 1 for secondary
	Uint8		type;			// 0 for ATA, 1 for ATAPI
	char		serial[21];		// The serial number of the drive
	char		model[41];		// The model number of the drive
} ATADevice;

typedef Uint8 ATASector[512];

// GLOBALS /////////////////////////////////////////////////////////////////
ATADevice ata_devices[10];
Uint8 ata_device_count; 
	
// FUNCTIONS ///////////////////////////////////////////////////////////////
void _testGetPCIInfo(void);
void _ata_probe(Uint16 bus, Uint16 device, Uint16 func);
void _ata_initialize(ATAController *cont, Uint16 bus, Uint16 device, Uint16 func);
Uint8 _ata_read_reg(ATAChannel channel, ATAReg reg);
void _ata_write_reg(ATAChannel channel, ATAReg reg, Uint8 payload);
Uint16 _ata_get_bar(Uint16 bus, Uint16 device, Uint16 func, Uint16 offset);
void _ata_read_sector(ATADevice dev, Uint64 lba, ATASector *d);
void _ata_write_sector(ATADevice dev, Uint64 lba, ATASector *s);

void _ata_wait( void );
void _ata_wait_bsy(ATAChannel channel);

#endif
