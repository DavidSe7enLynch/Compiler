/* variable 'a' allocated vreg 10 */
/* variable 'b' allocated vreg 11 */
/* variable 'c' allocated vreg 12 */
/* Function 'main' uses 0 bytes of memory and 13 virtual registers */
/* Function 'main' uses 56 total bytes of memory storage for vregs */
/* Function 'main': placing vreg storage at offset -56 from %rbp */
/* Function 'main': 64 bytes of local storage allocated in stack frame */
	.section .text

	.globl main
main:
	pushq    %rbp
	movq     %rsp, %rbp
	subq     $64, %rsp
	movl     $2, -32(%rbp)
	movl     -32(%rbp), %r10d
	movl     %r10d, -56(%rbp)
	movl     $3, -32(%rbp)
	movl     -32(%rbp), %r10d
	movl     %r10d, -48(%rbp)
	movl     $1, -32(%rbp)
	movl     -32(%rbp), %r10d
	movl     %r10d, -40(%rbp)
	movl     $1, -24(%rbp)
	movl     -56(%rbp), %r10d
	addl     -24(%rbp), %r10d
	movl     %r10d, -16(%rbp)
	movl     -16(%rbp), %r10d
	cmpl     -48(%rbp), %r10d
	sete     %r10b
	movzbl   %r10b, %r11d
	movl     %r11d, -8(%rbp)
	cmpl     $0, -8(%rbp)
	je       .L0
	movl     $0, -32(%rbp)
	movl     -32(%rbp), %r10d
	movl     %r10d, -40(%rbp)
.L0:
	movl     -40(%rbp), %eax
	jmp      .Lmain_return
.Lmain_return:
	addq     $64, %rsp
	popq     %rbp
	ret      
