#ifndef PCI_H
#define PCI_H

/**
 * Port information for talking to the PCI bus
 * Source: http://wiki.osdev.org/PCI#Configuration_Mechanism_.231
 */
#define CONF_ACC	0xCF8
#define CONFIG_ACCESS	CONF_ACC
#define	CONF_DAT	0xCFC
#define CONFIG_DATA		CONF_DAT

/**
 * Class Code information for identifying the tpye of device found
 * Source: http://wiki.osdev.org/PCI#Configuration_Mechanism_.231
 */
#define NO_CLASS	0x00
#define	MASS_STOR	0x01
#define NET_CONT	0x02
#define DISP_CONT	0x03

#define INVALID		0xFFFF

void _pci_init( void );

// Config read functions
Uint32 _pci_config_readl( Uint16 bus, Uint16 slot, Uint16 func, Uint16 offset );
Uint16 _pci_config_read( Uint16 bus, Uint16 slot, Uint16 func, Uint16 offset );
Uint8  _pci_config_readb( Uint16 bus, Uint16 slot, Uint16 func, Uint16 offset );

// Config write functions
void _pci_config_write( Uint16 bus, Uint16 device, Uint16 func, Uint16 offset,
		Uint16 payload );
void _pci_config_writel( Uint16 bus, Uint16 device, Uint16 func, Uint16 offset,
		Uint32 payload );

// Helper functions
Uint32 _pci_config_get_address( Uint16 bus, Uint16 device, Uint16 func, 
		Uint16 offset );

#endif
