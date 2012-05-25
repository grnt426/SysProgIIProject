/*
** File:	vmemA.S
**
** Author:	Sean McDermott
**
** Contributor:
**
** Description:	This file contains all the assembly function for the vmem suite including
** 		vmem, vmemL2, vmem_isr
*/

#define	__KERNEL__20113__
#define	__ASM__20113__

	.globl	_vmem_turnon
_vmem_turnon:
        enter   $0,$0
	movl 	%cr4, %ebx	// get cr4 
	or 	$0x00000010, %ebx	//turn bits on for PSE which enables 4MB pages
	movl 	%ebx, %cr4	// set the new cr4
	movl    8(%ebp),%eax	// Get the address of the page directory into %eax,
	movl	%eax, %cr3	// move the new addrress into cr3 as the fist page table
	movl	%cr0, %eax	// get cr0
	or	$0x80000000, %eax	// change bits to enable paging
	movl	%eax, %cr0	// set the cr0
	leave			
	ret

	.globl	_vmem_getcr0
_vmem_getcr0:
	movl	%cr0, %eax	// move the cr0 to the return 
	ret

	.globl	_vmem_bsf
_vmem_bsf:
	enter   $0,$0
	movl    8(%ebp),%ebx 	// get the first argument 
	bsf	%ebx, %eax	// search from the argument for the least significant bit set to 1, and put the answer into the return
	leave
	ret

	.globl	_vmeml2_change_page
_vmeml2_change_page:
        enter   $0,$0
	movl    8(%ebp),%eax	// Get the address of the page directory into %eax,
	movl	%eax, %cr3	// set the the new page directory
	leave			
	ret
	
	.globl	_isr_vmem_getcr2
_isr_vmem_getcr2:
	movl	%cr2, %eax	// get the current cr2 and put it into the return
	ret

	.globl	_isr_vmem_getcr3
_isr_vmem_getcr3:
	movl	%cr3, %eax	// get the current cr3 and put it into the return
	ret

	.globl _isr_vmem_geteip
_isr_vmem_geteip:
	movl 4(%esp), %eax 	// get the current eip and store it into the return
	ret
	
	.globl _isr_vmem_getcs
_isr_vmem_getcs:
	movl 8(%esp), %eax	// get the current code segment and put it into the return
	ret
