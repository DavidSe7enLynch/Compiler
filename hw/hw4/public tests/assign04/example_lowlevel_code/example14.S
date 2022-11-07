/* variable 'arr' allocated 4 bytes of storage at offset 0 */
/* variable 'sum' allocated vreg 10 */
/* Function 'main' uses 4 bytes of memory and 11 virtual registers */
/* Function 'main': placing memory variables at offset -8 from %rbp */
/* Function 'main' uses 72 total bytes of memory storage for vregs */
/* Function 'main': placing vreg storage at offset -80 from %rbp */
/* Function 'main': 80 bytes of local storage allocated in stack frame */
	.section .text

	.globl main
main:
	pushq    %rbp
	movq     %rsp, %rbp
	subq     $80, %rsp
	leaq     -8(%rbp), %r10
	movq     %r10, -72(%rbp)
	movl     $0, -64(%rbp)
	movl     -64(%rbp), %r10d
	movslq   %r10d, %r10
	movq     %r10, -56(%rbp)
	movq     -56(%rbp), %r10
	imulq    $4, %r10
	movq     %r10, -48(%rbp)
	movq     -72(%rbp), %r10
	addq     -48(%rbp), %r10
	movq     %r10, -40(%rbp)
	movl     $2, -32(%rbp)
	movq     -40(%rbp), %r11
	movl     -32(%rbp), %r10d
	movl     %r10d, (%r11)
	leaq     -8(%rbp), %r10
	movq     %r10, -72(%rbp)
	movl     $0, -64(%rbp)
	movl     -64(%rbp), %r10d
	movslq   %r10d, %r10
	movq     %r10, -56(%rbp)
	movq     -56(%rbp), %r10
	imulq    $4, %r10
	movq     %r10, -48(%rbp)
	movq     -72(%rbp), %r10
	addq     -48(%rbp), %r10
	movq     %r10, -40(%rbp)
	movl     $1, -32(%rbp)
	movq     -40(%rbp), %r11
	movl     (%r11), %r10d
	movl     %r10d, -16(%rbp)
	movl     -16(%rbp), %r10d
	addl     -32(%rbp), %r10d
	movl     %r10d, -24(%rbp)
	movl     -24(%rbp), %r10d
	movl     %r10d, -80(%rbp)
	movl     -80(%rbp), %eax
	jmp      .Lmain_return
.Lmain_return:
	addq     $80, %rsp
	popq     %rbp
	ret      
