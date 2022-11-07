/* variable 's' allocated vreg 10 */
/* Function 'main' uses 0 bytes of memory and 10 virtual registers */
	.section .rodata

_str0: .string "Hello, world\n"
/* Function 'main' uses 16 total bytes of memory storage for vregs */
/* Function 'main': placing vreg storage at offset -16 from %rbp */
/* Function 'main': 16 bytes of local storage allocated in stack frame */
	.section .text

	.globl main
main:
	pushq    %rbp
	movq     %rsp, %rbp
	subq     $16, %rsp
	movq     $_str0, -16(%rbp)
	movq     -16(%rbp), %rdi
	call     print_str
	movl     $0, -8(%rbp)
	movl     -8(%rbp), %eax
	jmp      .Lmain_return
.Lmain_return:
	addq     $16, %rsp
	popq     %rbp
	ret      
