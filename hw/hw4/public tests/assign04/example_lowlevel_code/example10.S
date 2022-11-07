/* variable 'a' allocated 4 bytes of storage at offset 0 */
/* variable 'p' allocated vreg 10 */
/* Function 'main' uses 4 bytes of memory and 11 virtual registers */
/* Function 'main': placing memory variables at offset -8 from %rbp */
/* Function 'main' uses 24 total bytes of memory storage for vregs */
/* Function 'main': placing vreg storage at offset -32 from %rbp */
/* Function 'main': 32 bytes of local storage allocated in stack frame */
	.section .text

	.globl main
main:
	pushq    %rbp
	movq     %rsp, %rbp
	subq     $32, %rsp
	leaq     -8(%rbp), %r10
	movq     %r10, -24(%rbp)
	movq     -24(%rbp), %r10
	movq     %r10, -32(%rbp)
	movl     $42, -24(%rbp)
	movq     -32(%rbp), %r11
	movl     -24(%rbp), %r10d
	movl     %r10d, (%r11)
	leaq     -8(%rbp), %r10
	movq     %r10, -16(%rbp)
	movq     -16(%rbp), %r11
	movl     (%r11), %eax
	jmp      .Lmain_return
.Lmain_return:
	addq     $32, %rsp
	popq     %rbp
	ret      
