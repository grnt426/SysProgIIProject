#
# SCCS ID: @(#)Makefile	1.14	4/5/12
#
# Makefile to control the compiling, assembling and linking of
# standalone programs in the DSL.  Used for both 4003-406 and
# 4003-506 (with appropriate tweaking).
#

#
# User supplied files
#
U_C_SRC = clock.c klibc.c pcbs.c queues.c scheduler.c sio.c stacks.c syscalls.c system.c ulibc.c users.c mouse.c keyboard.c vga_dr.c win_man.c gl.c vmem.c vmemL2.c vmem_isr.c vmem_ref.c fs.c ata.c pci.c ufs.c gl_print.c printf.c string.c
U_C_OBJ = clock.o klibc.o pcbs.o queues.o scheduler.o sio.o stacks.o syscalls.o system.o ulibc.o users.o mouse.o keyboard.o vga_dr.o win_man.o gl.o vmem.o vmemL2.o vmem_isr.o vmem_ref.o fs.o ata.o pci.o ufs.o gl_print.o printf.o string.o
U_S_SRC = klibs.S ulibs.S vmemA.S
U_S_OBJ = klibs.o ulibs.o vmemA.o
U_LIBS	=

#
# User compilation/assembly definable options
#
#	ISR_DEBUGGING_CODE	include context restore debugging code
#	CLEAR_BSS_SEGMENT	include code to clear all BSS space
#	SP2_CONFIG		enable SP2-specific startup variations
#	REPORT_MYSTERY_INTS	print a message on interrupt 0x27
#
USER_OPTIONS = -DCLEAR_BSS_SEGMENT -DSP2_CONFIG -DISR_DEBUGGING_CODE

#
# YOU SHOULD NOT NEED TO CHANGE ANYTHING BELOW THIS POINT!!!
#
# Compilation/assembly control
#

#
# We only want to include from the current directory and ~wrc/include
#
INCLUDES = -I. -I/home/fac/wrc/include

#
# Compilation/assembly/linking commands and options
#
CPP = cpp
# CPPFLAGS = $(USER_OPTIONS) -nostdinc -I- $(INCLUDES)
CPPFLAGS = $(USER_OPTIONS) -nostdinc $(INCLUDES)

CC = gcc
CFLAGS = -fno-stack-protector -fno-builtin -Wall -Wstrict-prototypes $(CPPFLAGS)

AS = as
ASFLAGS =

LD = ld

#		
# Transformation rules - these ensure that all compilation
# flags that are necessary are specified
#
# Note use of 'cpp' to convert .S files to temporary .s files: this allows
# use of #include/#define/#ifdef statements. However, the line numbers of
# error messages reflect the .s file rather than the original .S file. 
# (If the .s file already exists before a .S file is assembled, then
# the temporary .s file is not deleted.  This is useful for figuring
# out the line numbers of error messages, but take care not to accidentally
# start fixing things by editing the .s file.)
#

.SUFFIXES:	.S .b

.c.s:
	$(CC) $(CFLAGS) -S $*.c

.S.s:
	$(CPP) $(CPPFLAGS) -o $*.s $*.S

.S.o:
	$(CPP) $(CPPFLAGS) -o $*.s $*.S
	$(AS) -o $*.o $*.s -a=$*.lst
	$(RM) -f $*.s

.s.b:
	$(AS) -o $*.o $*.s -a=$*.lst
	$(LD) -Ttext 0x0 -s --oformat binary -e begtext -o $*.b $*.o

.c.o:
	$(CC) $(CFLAGS) -c $*.c

# Binary/source file for system bootstrap code

BOOT_OBJ = bootstrap.b
BOOT_SRC = bootstrap.S vga_dr_S.S

# Assembly language object/source files

S_OBJ = startup.o isr_stubs.o $(U_S_OBJ)
S_SRC =	startup.S isr_stubs.S $(U_S_SRC)

# C object/source files

C_OBJ =	c_io.o support.o $(U_C_OBJ)
C_SRC =	c_io.c support.c $(U_C_SRC)

# Collections of files

OBJECTS = $(S_OBJ) $(C_OBJ)

SOURCES = $(BOOT_SRC) $(S_SRC) $(C_SRC)

#
# Targets for remaking bootable image of the program
#
# Default target:  usb.image
#

usb.image: bootstrap.b prog.b prog.nl BuildImage #prog.dis 
	./BuildImage -d usb -o usb.image -b bootstrap.b prog.b 0x10000

floppy.image: bootstrap.b prog.b prog.nl BuildImage #prog.dis 
	./BuildImage -d floppy -o floppy.image -b bootstrap.b prog.b 0x10000

prog.out: $(OBJECTS)
	$(LD) -o prog.out $(OBJECTS)

prog.o:	$(OBJECTS)
	$(LD) -o prog.o -Ttext 0x10000 $(OBJECTS) $(U_LIBS)

prog.b:	prog.o
	$(LD) -o prog.b -s --oformat binary -Ttext 0x10000 prog.o

#
# Targets for copying bootable image onto boot devices
#

floppy:	floppy.image
	dd if=floppy.image of=/dev/fd0

usb:	usb.image
	dd if=usb.image of=/local/devices/disk

#
# Special rule for creating the modification and offset programs
#
# These are required because we don't want to use the same options
# as for the standalone binaries.
#

BuildImage:	BuildImage.c
	$(CC) -o BuildImage BuildImage.c

Offsets:	Offsets.c
	$(CC) $(INCLUDES) -o Offsets Offsets.c

#
# Clean out this directory
#

clean:
	rm -f *.nl *.lst *.b *.o *.image *.dis BuildImage Offsets

#
# Create a printable namelist from the prog.o file
#
# options to 'nm':
#	B	use (traditional) BSD format
#	n	sort by addresses, not by names
#	g	only global symbols
#

prog.nl: prog.o
#	nm -Bng prog.o | pr -w80 -3 > prog.nl
	nm -Bn prog.o | pr -w80 -3 > prog.nl

#
# Generate a disassembly
#
# options to 'objdump':
#	d	disassemble all text sections
#

prog.dis: prog.o
#	dis prog.o > prog.dis
	objdump -d prog.o > prog.dis

#
#       makedepend is a program which creates dependency lists by
#       looking at the #include lines in the source files
#

depend:
	makedepend $(INCLUDES) $(SOURCES)

# DO NOT DELETE THIS LINE -- make depend depends on it.

bootstrap.o: bootstrap.h vga_dr_S.S vga_define.h
vga_dr_S.o: vga_define.h
startup.o: bootstrap.h
isr_stubs.o: bootstrap.h
ulibs.o: syscalls.h headers.h queues.h /home/fac/wrc/include/x86arch.h
c_io.o: gl_print.h headers.h printf.h c_io.h startup.h support.h
c_io.o: /home/fac/wrc/include/x86arch.h
support.o: startup.h headers.h support.h c_io.h
support.o: /home/fac/wrc/include/x86arch.h bootstrap.h sio.h queues.h
support.o: string.h gl.h gl_print.h
clock.o: headers.h /home/fac/wrc/include/x86arch.h startup.h clock.h pcbs.h
clock.o: stacks.h queues.h scheduler.h sio.h syscalls.h
klibc.o: headers.h
pcbs.o: headers.h queues.h pcbs.h clock.h stacks.h
queues.o: headers.h pcbs.h clock.h stacks.h queues.h
scheduler.o: headers.h vmemL2.h scheduler.h pcbs.h clock.h stacks.h queues.h
sio.o: headers.h sio.h queues.h pcbs.h clock.h stacks.h scheduler.h system.h
sio.o: startup.h /home/fac/wrc/include/uart.h /home/fac/wrc/include/x86arch.h
stacks.o: headers.h queues.h stacks.h
syscalls.o: headers.h pcbs.h clock.h stacks.h scheduler.h queues.h sio.h
syscalls.o: syscalls.h /home/fac/wrc/include/x86arch.h system.h vmemL2.h
syscalls.o: vmem.h startup.h keyboard.h
system.o: headers.h startup.h system.h pcbs.h clock.h stacks.h bootstrap.h
system.o: syscalls.h queues.h /home/fac/wrc/include/x86arch.h sio.h
system.o: scheduler.h vga_dr.h gl.h win_man.h vmem.h vmemL2.h vmem_isr.h
system.o: vmem_ref.h pci.h fs.h ata.h users.h keyboard.h ulib.h types.h
ulibc.o: headers.h
users.o: headers.h users.h keyboard.h queues.h pcbs.h clock.h stacks.h gl.h
users.o: pci.h ufs.h fs.h ata.h string.h mouse.h gl_print.h syscalls.h
users.o: /home/fac/wrc/include/x86arch.h
mouse.o: headers.h startup.h ps2.h mouse.h win_man.h
keyboard.o: headers.h ps2.h system.h pcbs.h clock.h stacks.h startup.h
keyboard.o: queues.h scheduler.h ulib.h types.h win_man.h keyboard.h vmemL2.h
keyboard.o: vmem_isr.h gl_print.h
vga_dr.o: headers.h vga_dr.h vga_define.h
win_man.o: headers.h win_man.h vga_dr.h font_define.h gl.h c_io.h vmemL2.h
win_man.o: klib.h
gl.o: gl.h headers.h win_man.h vga_dr.h font.h font_define.h gl_print.h
gl.o: c_io.h
vmem.o: startup.h headers.h vmem.h
vmemL2.o: vmem.h headers.h vmemL2.h
vmem_isr.o: vmem_isr.h headers.h sio.h queues.h
vmem_ref.o: vmem_ref.h headers.h vmemL2.h
fs.o: headers.h pci.h startup.h ata.h fs.h
ata.o: headers.h pci.h startup.h ata.h
pci.o: headers.h startup.h pci.h ata.h fs.h ufs.h
ufs.o: ufs.h fs.h headers.h ata.h
gl_print.o: gl_print.h headers.h gl.h font_define.h
printf.o: printf.h gl_print.h headers.h
string.o: string.h headers.h
