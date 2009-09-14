#include <arch-x86_32.h>


.section __xen_guest
	.ascii	"GUEST_OS=Hacking_Xen_Example"
	.ascii	",XEN_VER=xen-3.0"
	.ascii	",VIRT_BASE=0x0"
	.ascii	",ELF_PADDR_OFFSET=0x0"
	.ascii	",HYPERCALL_PAGE=0x2"
	.ascii	",PAE=yes"
	.ascii	",LOADER=generic"
	.byte	0
.text

.globl _start, shared_info, hypercall_page

_start:
        cld
        lss stack_start,%esp
        push %esi
        call start_kernel

stack_start:
	.long stack+8192, FLAT_KERNEL_SS

        /* Unpleasant -- the PTE that maps this page is actually overwritten */
        /* to map the real shared-info page! :-)                             */
        .org 0x1000
shared_info:
        .org 0x2000

hypercall_page:
        .org 0x3000

ES		= 0x20
ORIG_EAX	= 0x24
EIP		= 0x28
CS		= 0x2C


#define ENTRY(X) .globl X ; X:


#define SAVE_ALL \
	cld; \
	pushl %es; \
	pushl %ds; \
	pushl %eax; \
	pushl %ebp; \
	pushl %edi; \
	pushl %esi; \
	pushl %edx; \
	pushl %ecx; \
	pushl %ebx; \
	movl $(FLAT_KERNEL_DS),%edx; \
	movl %edx,%ds; \
	movl %edx,%es;


#define RESTORE_ALL	\
	popl %ebx;	\
	popl %ecx;	\
	popl %edx;	\
	popl %esi;	\
	popl %edi;	\
	popl %ebp;	\
	popl %eax;	\
	popl %ds;	\
	popl %es;	\
	addl $4,%esp;	\
	iret;


ENTRY (hypervisor_callback)
        pushl   %eax
        SAVE_ALL
        movl    EIP(%esp), %eax
        cmpl    $scrit, %eax
        jb      11f
        cmpl    $ecrit, %eax
        jb      critical_region_fixup
11:     push    %esp
        call    do_hypervisor_callback
        add     $4, %esp
        movl    HYPERVISOR_shared_info, %esi
        xorl    %eax,%eax
        movb    CS(%esp),%cl
	test    $2,%cl          # slow return to ring 2 or 3
        jne     safesti

safesti:
        movb    $0,1(%esi)     # reenable event callbacks

scrit:  /**** START OF CRITICAL REGION ****/
        testb    $0xFF,(%esi)
        jnz     14f              # process more events if necessary...
        RESTORE_ALL
14:     movb    $1,1(%esi)
        jmp     11b
ecrit:  /**** END OF CRITICAL REGION ****/

# [How we do the fixup]. We want to merge the current stack frame with the
# just-interrupted frame. How we do this depends on where in the critical
# region the interrupted handler was executing, and so how many saved
# registers are in each frame. We do this quickly using the lookup table
# 'critical_fixup_table'. For each byte offset in the critical region, it
# provides the number of bytes which have already been popped from the
# interrupted stack frame.

critical_region_fixup:
        addl     $critical_fixup_table-scrit,%eax
        movzbl  (%eax),%eax    # %eax contains num bytes popped
        mov     %esp,%esi
        add     %eax,%esi        # %esi points at end of src region
        mov     %esp,%edi
        add     $0x34,%edi       # %edi points at end of dst region
        mov     %eax,%ecx
        shr     $2,%ecx          # convert words to bytes
        je      16f              # skip loop if nothing to copy
15:     subl    $4,%esi          # pre-decrementing copy loop
        subl    $4,%edi
        movl    (%esi),%eax
        movl    %eax,(%edi)
        loop    15b
16:     movl    %edi,%esp        # final %edi is top of merged stack
        jmp     11b

critical_fixup_table:
        .byte   0x00,0x00,0x00                  # testb $0xff,(%esi)
        .byte   0x00,0x00                       # jne  14f
        .byte   0x00                            # pop  %ebx
        .byte   0x04                            # pop  %ecx
        .byte   0x08                            # pop  %edx
        .byte   0x0c                            # pop  %esi
        .byte   0x10                            # pop  %edi
        .byte   0x14                            # pop  %ebp
        .byte   0x18                            # pop  %eax
        .byte   0x1c                            # pop  %ds
        .byte   0x20                            # pop  %es
        .byte   0x24,0x24,0x24                  # add  $4,%esp
        .byte   0x28                            # iret
        .byte   0x00,0x00,0x00,0x00             # movb $1,1(%esi)
        .byte   0x00,0x00                       # jmp  11b



ENTRY (failsafe_callback)
        pop     %ds
        pop     %es
        pop     %fs
        pop     %gs
        iret