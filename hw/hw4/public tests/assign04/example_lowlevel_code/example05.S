/* variable 'a' allocated vreg 10 */
/* variable 'b' allocated vreg 11 */
/* variable 'result' allocated vreg 12 */
/* Function 'sum' uses 0 bytes of memory and 13 virtual registers */
/* variable 'x' allocated vreg 10 */
/* variable 'y' allocated vreg 11 */
/* variable 'z' allocated vreg 12 */
/* Function 'main' uses 0 bytes of memory and 13 virtual registers */
/* Function 'sum' uses 32 total bytes of memory storage for vregs */
/* Function 'sum': placing vreg storage at offset -32 from %rbp */
/* Function 'sum': 32 bytes of local storage allocated in stack frame */
	.section .text

	.globl sum
sum:
	pushq    %rbp
	movq     %rsp, %rbp
	subq     $32, %rsp
	movl     %edi, -32(%rbp)
	movl     %esi, -24(%rbp)
	movl     -32(%rbp), %r10d
	addl     -24(%rbp), %r10d
	movl     %r10d, -8(%rbp)
	movl     -8(%rbp), %r10d
	movl     %r10d, -16(%rbp)
	movl     -16(%rbp), %eax
	jmp      .Lsum_return
.Lsum_return:
	addq     $32, %rsp
	popq     %rbp
	ret      
/* Function 'main' uses 32 total bytes of memory storage for vregs */
/* Function 'main': placing vreg storage at offset -32 from %rbp */
/* Function 'main': 32 bytes of local storage allocated in stack frame */

	.globl main
main:
	pushq    %rbp
	movq     %rsp, %rbp
	subq     $32, %rsp
	movl     $2, -8(%rbp)
	movl     -8(%rbp), %r10d
	movl     %r10d, -32(%rbp)
	movl     $3, -8(%rbp)
	movl     -8(%rbp), %r10d
	movl     %r10d, -24(%rbp)
	movl     -32(%rbp), %edi
	movl     -24(%rbp), %esi
	call     sum
	movl     %eax, -8(%rbp)
	movl     -8(%rbp), %r10d
	movl     %r10d, -16(%rbp)
	movl     -16(%rbp), %eax
	jmp      .Lmain_return
.Lmain_return:
	addq     $32, %rsp
	popq     %rbp
	ret      
