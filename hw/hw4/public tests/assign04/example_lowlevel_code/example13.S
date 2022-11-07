/* variable 'x' allocated vreg 10 */
/* variable 'y' allocated vreg 11 */
/* variable 'p' allocated 8 bytes of storage at offset 0 */
/* variable 'q' allocated vreg 10 */
/* Function 'main' uses 8 bytes of memory and 11 virtual registers */
/* Function 'main': placing memory variables at offset -8 from %rbp */
/* Function 'main' uses 88 total bytes of memory storage for vregs */
/* Function 'main': placing vreg storage at offset -96 from %rbp */
/* Function 'main': 96 bytes of local storage allocated in stack frame */
	.section .text

	.globl main
main:
	pushq    %rbp
	movq     %rsp, %rbp
	subq     $96, %rsp
	leaq     -8(%rbp), %r10
	movq     %r10, -88(%rbp)
	movq     -88(%rbp), %r10
	movq     %r10, -96(%rbp)
	movq     $0, -88(%rbp)
	movq     -96(%rbp), %r10
	addq     -88(%rbp), %r10
	movq     %r10, -80(%rbp)
	movl     $2, -72(%rbp)
	movq     -80(%rbp), %r11
	movl     -72(%rbp), %r10d
	movl     %r10d, (%r11)
	movq     $4, -88(%rbp)
	movq     -96(%rbp), %r10
	addq     -88(%rbp), %r10
	movq     %r10, -80(%rbp)
	movl     $3, -72(%rbp)
	movq     -80(%rbp), %r11
	movl     -72(%rbp), %r10d
	movl     %r10d, (%r11)
	movq     $0, -64(%rbp)
	movq     -96(%rbp), %r10
	addq     -64(%rbp), %r10
	movq     %r10, -56(%rbp)
	movq     $4, -48(%rbp)
	movq     -96(%rbp), %r10
	addq     -48(%rbp), %r10
	movq     %r10, -40(%rbp)
	movq     -56(%rbp), %r11
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
	addq     $96, %rsp
	popq     %rbp
	ret      
