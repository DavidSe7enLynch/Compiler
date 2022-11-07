/* variable 'a' allocated 4 bytes of storage at offset 0 */
/* variable 'p' allocated 8 bytes of storage at offset 8 */
/* variable 'pp' allocated vreg 10 */
/* Function 'main' uses 16 bytes of memory and 11 virtual registers */
/* Function 'main': placing memory variables at offset -16 from %rbp */
/* Function 'main' uses 32 total bytes of memory storage for vregs */
/* Function 'main': placing vreg storage at offset -48 from %rbp */
/* Function 'main': 48 bytes of local storage allocated in stack frame */
	.section .text

	.globl main
main:
	pushq    %rbp
	movq     %rsp, %rbp
	subq     $48, %rsp
	leaq     -8(%rbp), %r10
	movq     %r10, -40(%rbp)
	leaq     -16(%rbp), %r10
	movq     %r10, -32(%rbp)
	movq     -40(%rbp), %r11
	movq     -32(%rbp), %r10
	movq     %r10, (%r11)
	leaq     -8(%rbp), %r10
	movq     %r10, -40(%rbp)
	movq     -40(%rbp), %r10
	movq     %r10, -48(%rbp)
	movq     -48(%rbp), %r11
	movq     (%r11), %r10
	movq     %r10, -40(%rbp)
	movl     $17, -32(%rbp)
	movq     -40(%rbp), %r11
	movl     -32(%rbp), %r10d
	movl     %r10d, (%r11)
	leaq     -16(%rbp), %r10
	movq     %r10, -24(%rbp)
	movq     -24(%rbp), %r11
	movl     (%r11), %eax
	jmp      .Lmain_return
.Lmain_return:
	addq     $48, %rsp
	popq     %rbp
	ret      
