/*
** SCCS ID: @(#)users.c 1.1 4/5/12
**
** File:    ata.c
** Author:  Benjamin Russell
** Description: User routines.
*/

// REQUIRED FILES //////////////////////////////////////////////////////////

#include "headers.h"
#include "pci.h"
#include "startup.h"
#include "ata.h"

// DEFINES /////////////////////////////////////////////////////////////////

// FUNCTIONS ///////////////////////////////////////////////////////////////

/**
 * Reads from the PCI controller to get the base address register (BAR) or the
 * specified ata register. Really it just grabs a long from a register and
 * does a little bit of bit twiddling with it.
 * @param	bus		Uint16	The pci bus that the ATA controller is on
 * @param	device	Uint16	The device number of the ATA controller
 * @param	func	Uint16	The function number of the ATA controller
 * @param	offset	Uint16	The offset into the PCI config space that yields
 *							the BAR you're looking for
 * @return	Uint16	The value of that BAR register in the PCI config space
 */
Uint16 _ata_get_bar(Uint16 bus, Uint16 device, Uint16 func, Uint16 offset) {
	// Read the register at the offset
	Uint32 reg = _pci_config_readl(bus, device, func, offset);
	// Return the address after bitmasking
	// The format of the register looks like:
	//	31:16	- Reserved
	//	15:3	- Base Address		
	//	 2:1	- Reserved
	//	   0	- Hardwired to 1	<- Not needed, so AND it off
	return (reg & 0xFFFFFFFE);
}


/**
 * Reads a byte from the specified ide channel register. THAR BE MAGIC IN HERE.
 * @param	ATAhannel	channel	The channel to read from
 * @param	ATAReg 		reg		The register on the channel to read from
 * @source	http://wiki.osdev.org/IDE
 */
Uint8 _ata_read_reg(ATAChannel channel, ATAReg reg) {
	Uint8 result;
	if(reg > 0x07 && reg < 0x0C) {
		_ata_write_reg(channel, ATA_REG_CONTROL, 0x80 | ATA_NOINT);
	}

	if(reg < 0x08) {
		result = __inb(channel.command + reg - 0x00);
	} else if(reg < 0x0C) {
		result = __inb(channel.command  + reg - 0x06);
	} else if(reg < 0x0E) {
		result = __inb(channel.control  + reg - 0x0A);
	} else if (reg < 0x16) {
		result = __inb(channel.busmast + reg - 0x0E);
	}
   
	if(reg > 0x07 && reg < 0x0C) {
		_ata_write_reg(channel, ATA_REG_CONTROL, ATA_NOINT);
	}
   
	return result;
}

/**
 * Reads a word from the ATA data register on the desired ATA channel
 * @param	ATAChannel	channel	The channel to read data from
 * @return	Uint16	A word-worth of data from the data register
 */
Uint16 _ata_read_data(ATAChannel channel) {
	return __inw(channel.command);
}

/**
 * Writes a word to the ATA data register on the desired ATA channel
 * @param	ATAChannel	channel	The channel to write data to
 * @param	Uint16		payload	The word of data to write to data
 */
void _ata_write_data(ATAChannel channel, Uint16 payload) {
	__outw(channel.command, payload);
}

/**
 * Writes a byte to the specified ATA channel reg. THAR BE MAGIC HERE.
 * @param	ATAChannel 	channel	The channel to write to
 * @param	ATARegs 	reg		The register on the channel to write to
 * @param	Uint8 		payload	The byte to write the the register
 * @source	http://wiki.osdev.org/IDE
 */
void _ata_write_reg(ATAChannel channel, ATAReg reg, Uint8 payload) {
	if(reg > 0x07 && reg < 0x0C) {
		_ata_write_reg(channel, ATA_REG_CONTROL, 0x80 | ATA_NOINT);
	} 

	if(reg < 0x08) {
		__outb(channel.command  + reg - 0x00, payload);
	} else if(reg < 0x0C) {
		__outb(channel.command  + reg - 0x06, payload);
	} else if(reg < 0x0E) {
		__outb(channel.control  + reg - 0x0A, payload);
    } else if(reg < 0x16) {
		__outb(channel.busmast + reg - 0x0E, payload);
	}

	if(reg > 0x07 && reg < 0x0C) {
		_ata_write_reg(channel, ATA_REG_CONTROL, ATA_NOINT);
	}
}

/**
 * Initializes a ATAController struct that resides on a specified PCI bus by
 * reading and storing all the BARs of the controller.
 * @param	ATAController*	cont	Address to the controller to initialize
 * @param	Uint16			bus		The PCI bus the controller lives on
 * @param	Uint16			device	The device number of the controller
 * @param	Uint16			func	The function number of the controller
 */
void _ata_initialize(ATAController *cont, Uint16 bus, Uint16 device, Uint16 func) {
	// Initialize the controller into IDE mode
	_pci_config_write(bus, device, func, ATA_PCI_REG_MAP, 0x00);

	// Store the bar addresses
	(*cont)[ATA_PORT_CHANPRI].command = _ata_get_bar(bus, device, func, ATA_PCI_REG_PCMD);
	(*cont)[ATA_PORT_CHANPRI].control = _ata_get_bar(bus, device, func, ATA_PCI_REG_PCTRL);
	(*cont)[ATA_PORT_CHANSEC].command = _ata_get_bar(bus, device, func, ATA_PCI_REG_SCMD);
	(*cont)[ATA_PORT_CHANSEC].control = _ata_get_bar(bus, device, func, ATA_PCI_REG_SCTRL);
	(*cont)[ATA_PORT_CHANPRI].busmast = _ata_get_bar(bus, device, func, ATA_PCI_REG_BMAST);
	(*cont)[ATA_PORT_CHANSEC].busmast = (*cont)[ATA_PORT_CHANPRI].busmast + 0x8;
}

/**
 * Probes an ATA controller at the PCI bus:device.function address
 * provided. Access the global list of ATA devices to store the info about them
 * @param	Uint16	bus		The PCI bus of the ATA controller
 * @param	Uint16	device	The PCI device number of the ATA controller
 * @param	Uint16	func	The PCI function number of the ATA controller
 */
void _ata_probe(Uint16 bus, Uint16 device, Uint16 func) {
	// Create a controller struct to assist in constructing the drives
	// Also create vars for storing individual device information
	ATAController cont;
	Uint8         chan;				// Primary vs. Secondary
	Uint8         dev;				// Master vs. Slave
	Uint16        identSpace[256];	// 256 words for the IDENTIFY command

	// Initialize the controller object
	_ata_initialize(&cont, bus, device, func);
	
	// IF the command port is 00, then we need to ignore this invalid device
	if(cont[ATA_PORT_CHANPRI].command == 0) {
		// @TODO: Set the 'default' BARs for this controller
		return;
	}

	// Turn off IRQ's for the channels
	//_ata_write_reg(cont[ATA_PORT_CHANPRI], ATA_REG_CONTROL, 2);
	//_ata_write_reg(cont[ATA_PORT_CHANSEC], ATA_REG_CONTROL, 2);
	
	// Iterate over the channels to probe
	for(chan = ATA_PORT_CHANPRI; chan <= ATA_PORT_CHANSEC; chan++) {
		// Iterate over each device on the channel
		for(dev = ATA_PORT_CHANMAST; dev <= ATA_PORT_CHANSLAV; dev++) {
			// Tell the controller which drive we want
			_ata_write_reg(cont[chan], ATA_REG_CONTROL, ATA_NOINT);
			_ata_write_reg(cont[chan], ATA_REG_DRIVESEL, 0xA0 | (dev << 4));
			_ata_wait();

			// Tell the device we want it's identity information
			_ata_write_reg(cont[chan], ATA_REG_COMMAND, ATA_CMD_IDENTIFY);
			_ata_wait();
			
			Uint8 status = _ata_read_reg(cont[chan], ATA_REG_STATUS);
			if((status & 0x40) == 0 || status & 0x01) {
				// Drive isn't there (or we don't want it)
				// DriveRDY bit is off || error bit is set
				continue;
			}

			// Read the identification data into the identSpace var
			Uint16 words;
			Uint16 word;
			for(words = 0; words < 256; words++) {
				// Read from the register
				word = _ata_read_data(cont[chan]);
				
				// Copy into the identspace
				identSpace[words] = word;
			}

			// Grab relevant data from the identify command
			// Command Set
			Uint16 commands = identSpace[82];
			//c_printf("word 82=0x%04x\n", commands);
			if(commands & 0x10) {
				// Device is ATAPI, skip it
				continue;
			}

			// Increment the drives we have
			Uint8 deviceId = ata_device_count;
			ata_device_count++;
			ata_devices[deviceId].channel = cont[chan];
			ata_devices[deviceId].device  = dev;

			// Serial String - copy byte by byte (20 bytes, 10 words)
			Uint8 m;
			for(m = 0; m < 10; m++) {
				// Copy high byte then low byte
				ata_devices[deviceId].serial[m * 2]     = (char)(identSpace[10 + m] >> 8);
				ata_devices[deviceId].serial[m * 2 + 1] = (char)identSpace[10 + m];
			}
			ata_devices[deviceId].serial[20] = 0x0;		// Null terminate

			// Model String - copy byte by byte (40 bytes, 20 words)
			for(m = 0; m < 20; m++) {
				// Copy high byten then low byte
				ata_devices[deviceId].model[m*2]   = (char)(identSpace[27+m]>>8);
				ata_devices[deviceId].model[m*2+1] = (char)identSpace[27+m];
			}
			ata_devices[deviceId].model[40] = 0x0;		// Null terminate

			// Size in Sectors - OR together the sector count
			// @TODO: See if <ATA-7 is supported to determine if LBA24 is needed
			Uint32 size = identSpace[101] << 16 | identSpace[100];
			ata_devices[deviceId].size = size;
		}	
	}
}

/**
 * Does a quick spin to make sure the controller switched to the proper device
 */
void _ata_wait( void ) {
	volatile int i;
	for(i = 0; i < 1000000; i++);
}

/**
 * Does a quick spin while waiting for the busy bit on the status register of
 * the device to not be set
 */
Uint8 _ata_wait_bsy(ATAChannel channel) {
	if(_ata_read_reg(channel, ATA_REG_STATUS) & ATA_STATUS_ERR) {
		return _ata_read_reg(channel, ATA_REG_ERROR);
	}

	// No errors. Wait til !busy
	while(_ata_read_reg(channel, ATA_REG_STATUS) & ATA_STATUS_BUSY);
	return ATA_STATUS_OK;
}



void _ata_read_sector(ATADevice *dev, Uint32 lba, ATASector *dest) {
	// Steb 1a) Tell the controller which drive we'd like to read from
	// E0 tells the drive we're doing LBA (and some obsolete bits?)
	_ata_write_reg(dev->channel, ATA_REG_DRIVESEL, 0xE0 | (dev->device << 4));
	_ata_wait();
	_ata_write_reg(dev->channel, ATA_REG_CONTROL, ATA_NOINT);	// Turn off interrupts

	// Step 1b) Tell the drive what sector we want to read
	// NOTE: We only support 48-bit LBA addressing for now
	// @TODO: Support 24-bit LBA
	_ata_write_reg(dev->channel, ATA_REG_LBA3, (lba & 0xFF000000) >> 24);
	_ata_write_reg(dev->channel, ATA_REG_LBA4, 0x0); // Ignoring the last two
	_ata_write_reg(dev->channel, ATA_REG_LBA5, 0x0); // regs 32-bit >= 2Tb
	_ata_write_reg(dev->channel, ATA_REG_SECCOUNT2, 0x0);
	_ata_write_reg(dev->channel, ATA_REG_LBA0,  lba & 0x000000FF);
	_ata_write_reg(dev->channel, ATA_REG_LBA1, (lba & 0x0000FF00) >> 8);
	_ata_write_reg(dev->channel, ATA_REG_LBA2, (lba & 0x00FF0000) >> 16);
	_ata_write_reg(dev->channel, ATA_REG_SECCOUNT1, 0x1);
	
	// Step 2) Tell the drive that we want to read sector
	_ata_write_reg(dev->channel, ATA_REG_COMMAND, ATA_CMD_READSECE);
	
	// Step 3) Wait until the drive isn't busy
	Uint8 status = _ata_wait_bsy(dev->channel);
	if(status != ATA_STATUS_OK) {
		// @TODO: error out
		return;
	}

	// Step 4) Read the data as words from the data reg
	Uint16 i, word;
	for(i = 0; i < 512; i += 2) {
		word = _ata_read_data(dev->channel);
		(*dest)[i+1] = word >> 8;
		(*dest)[i] = word;
	}

	// Step 5) Ack that we got the data by reading status
	status = _ata_wait_bsy(dev->channel);
	if(status != ATA_STATUS_OK) {
		// @TODO: Error out
	}
}

void _ata_write_sector(ATADevice *dev, Uint32 lba, ATASector *s) {
	// Step 1) Set up for the write
	// Step 1a) Tell the controller which device to write to
	//          Also turn off interrupts
	_ata_write_reg(dev->channel, ATA_REG_DRIVESEL, 0xE0 | (dev->device << 4));
	_ata_wait();
	_ata_write_reg(dev->channel, ATA_REG_CONTROL, ATA_NOINT);

	// Step 1b) Tell the device what sector to seek to
	_ata_write_reg(dev->channel, ATA_REG_LBA3, (lba & 0xFF000000) >> 24);
	_ata_write_reg(dev->channel, ATA_REG_LBA4, 0x0);
	_ata_write_reg(dev->channel, ATA_REG_LBA5, 0x0);
	_ata_write_reg(dev->channel, ATA_REG_SECCOUNT2, 0x0);
	_ata_write_reg(dev->channel, ATA_REG_LBA0,  lba & 0x000000FF);
	_ata_write_reg(dev->channel, ATA_REG_LBA1, (lba & 0x0000FF00) >> 8);
	_ata_write_reg(dev->channel, ATA_REG_LBA2, (lba & 0x00FF0000) >> 16);
	_ata_write_reg(dev->channel, ATA_REG_SECCOUNT1, 0x1);

	// Step 2) Issue the command
	_ata_write_reg(dev->channel, ATA_REG_COMMAND, ATA_CMD_WRITSECE);

	// Step 3) Wait until the device is ready
	Uint8 status = _ata_wait_bsy(dev->channel);
	if(status != ATA_STATUS_OK) {
		// @TODO: error out
		return;
	}

	// Step 4) Start sending words to the device via data register
	Uint16 i;
	for(i = 0; i < 512; i += 2) {
		_ata_write_data(dev->channel, ((*s)[i+1] << 8) | (*s)[i]);
	}

	// Step 5) Ack that we are done with data, then flush the cache
	status = _ata_wait_bsy(dev->channel);
	if(status != ATA_STATUS_OK) {
		// @TODO: Error out
	}
	
	_ata_write_reg(dev->channel, ATA_REG_COMMAND, ATA_CMD_FLUSHE);
	
	_ata_wait_bsy(dev->channel);
	status = _ata_wait_bsy(dev->channel);
	if(status != ATA_STATUS_OK) {
		// @TODO: Error out
	}		
}

void _ata_blank_sector(ATASector *s) {
	// Store zeros in the sector in MEMORY
	Uint16 i;
	for(i = 0; i < 512; i++) {
		(*s)[i] = 0x0;
	}
}

Uint32 _sector_get_long(ATASector *s, Uint16 offset) {
	return (*s)[offset+3]<<24|(*s)[offset+2]<<16|(*s)[offset+1]<<8|(*s)[offset];
}

Uint16 _sector_get_word(ATASector *s, Uint16 offset) {
	return (*s)[offset+1]<<8|(*s)[offset];
}

void _sector_put_long(ATASector *s, Uint16 offset, Uint32 payload) {
	(*s)[offset+3] = payload >> 24;
	(*s)[offset+2] = payload >> 16;
	(*s)[offset+1] = payload >> 8;
	(*s)[offset]   = payload;
}

void _sector_put_word(ATASector *s, Uint16 offset, Uint16 payload) {
	(*s)[offset+1] = payload >> 8;
	(*s)[offset]   = payload;
}
