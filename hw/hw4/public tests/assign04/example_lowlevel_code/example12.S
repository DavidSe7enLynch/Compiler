/* variable 'x' allocated vreg 10 */
/* variable 'y' allocated vreg 11 */
/* variable 'p' allocated 8 bytes of storage at offset 0 */
/* Function 'main' uses 8 bytes of memory and 10 virtual registers */
/* Function 'main': placing memory variables at offset -8 from %rbp */
/* Function 'main' uses 104 total bytes of memory storage for vregs */
/* Function 'main': placing vreg storage at offset -112 from %rbp */
/* Function 'main': 112 bytes of local storage allocated in stack frame */
	.section .text

	.globl main
main:
	pushq    %rbp
	movq     %rsp, %rbp
	subq     $112, %rsp
	leaq     -8(%rbp), %r10
	movq     %r10, -112(%rbp)
	movq     $0, -104(%rbp)
	movq     -112(%rbp), %r10
	addq     -104(%rbp), %r10
	movq     %r10, -96(%rbp)
	movl     $2, -88(%rbp)
	movq     -96(%rbp), %r11
	movl     -88(%rbp), %r10d
	movl     %r10d, (%r11)
	leaq     -8(%rbp), %r10
	movq     %r10, -112(%rbp)
	movq     $4, -104(%rbp)
	movq     -112(%rbp), %r10
	addq     -104(%rbp), %r10
	movq     %r10, -96(%rbp)
	movl     $3, -88(%rbp)
	movq     -96(%rbp), %r11
	movl     -88(%rbp), %r10d
	movl     %r10d, (%r11)
	leaq     -8(%rbp), %r10
	movq     %r10, -80(%rbp)
	movq     $0, -72(%rbp)
	movq     -80(%rbp), %r10
	addq     -72(%rbp), %r10
	movq     %r10, -64(%rbp)
	leaq     -8(%rbp), %r10
	movq     %r10, -56(%rbp)
	movq     $4, -48(%rbp)
	movq     -56(%rbp), %r10
	addq     -48(%rbp), %r10
	movq     %r10, -40(%rbp)
	movq     -64(%rbp), %r11
	movl     (%r11), %r10d
	movl     %r10d, -24(%rbp)
	movq     -40(%rbp), %r11
	movl     (%r11), %r10d
	movl     %r10d, -16(%rbp)
	movl     -24(%rbp), %r10d
	addl     -16(%rbp), %r10d
	movl     %r10d, -32(%rbp)
	movl     -32(%rbp), %eax
	jmp      .Lmain_return
.Lmain_return:
	addq     $112, %rsp
	popq     %rbp
	ret      
