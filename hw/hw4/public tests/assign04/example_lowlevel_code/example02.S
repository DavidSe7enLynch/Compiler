/* variable 'n' allocated vreg 10 */
/* variable 'i' allocated vreg 11 */
/* variable 'sum' allocated vreg 12 */
/* Function 'main' uses 0 bytes of memory and 13 virtual registers */
/* Function 'main' uses 48 total bytes of memory storage for vregs */
/* Function 'main': placing vreg storage at offset -48 from %rbp */
/* Function 'main': 48 bytes of local storage allocated in stack frame */
	.section .text

	.globl main
main:
	pushq    %rbp
	movq     %rsp, %rbp
	subq     $48, %rsp
	movl     $11, -24(%rbp)
	movl     -24(%rbp), %r10d
	movl     %r10d, -48(%rbp)
	movl     $1, -24(%rbp)
	movl     -24(%rbp), %r10d
	movl     %r10d, -40(%rbp)
	movl     $0, -24(%rbp)
	movl     -24(%rbp), %r10d
	movl     %r10d, -32(%rbp)
	jmp      .L1
.L0:
	movl     -32(%rbp), %r10d
	addl     -40(%rbp), %r10d
	movl     %r10d, -24(%rbp)
	movl     -24(%rbp), %r10d
	movl     %r10d, -32(%rbp)
	movl     $1, -24(%rbp)
	movl     -40(%rbp), %r10d
	addl     -24(%rbp), %r10d
	movl     %r10d, -16(%rbp)
	movl     -16(%rbp), %r10d
	movl     %r10d, -40(%rbp)
.L1:
	movl     -40(%rbp), %r10d
	cmpl     -48(%rbp), %r10d
	setle    %r10b
	movzbl   %r10b, %r11d
	movl     %r11d, -8(%rbp)
	cmpl     $0, -8(%rbp)
	jne      .L0
	movl     -32(%rbp), %eax
	jmp      .Lmain_return
.Lmain_return:
	addq     $48, %rsp
	popq     %rbp
	ret      
