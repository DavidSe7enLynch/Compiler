10: 41
11: 22
12: 10
13: 10
	.section .text

	.globl sum
sum:
	pushq    %r12
	pushq    %r13
	pushq    %r14
	pushq    %r15
	pushq    %rbp
	movq     %rsp, %rbp
	subq     $80, %rsp
	movq     %rdi, %r14
	movl     %esi, %r15
	movl     $0, -48(%rbp)
	movl     -48(%rbp), %r13
	movl     $0, -48(%rbp)
	movl     -48(%rbp), %r12
	jmp      .L1
.L0:
	movl     %r12, %r10d
	movslq   %r10d, %r10
	movq     %r10, -40(%rbp)
	movq     -40(%rbp), %r10
	imulq    $4, %r10
	movq     %r10, -32(%rbp)
	movq     -32(%rbp), %r10
	addq     %r14, %r10
	movq     %r10, -24(%rbp)
	movq     -24(%rbp), %r11
	movl     %r13, %r10d
	addl     (%r11), %r10d
	movl     %r10d, -16(%rbp)
	movl     -16(%rbp), %r13
	movl     $1, -48(%rbp)
	movl     %r12, %r10d
	addl     -48(%rbp), %r10d
	movl     %r10d, -40(%rbp)
	movl     -40(%rbp), %r12
.L1:
	movl     %r12, %r10d
	cmpl     %r15, %r10d
	setl     %r10b
	movzbl   %r10b, %r11d
	movl     %r11d, -32(%rbp)
	cmpl     $0, -32(%rbp)
	jne      .L0
	movl     %r13, %eax
	jmp      .Lsum_return
.Lsum_return:
	addq     $80, %rsp
	popq     %rbp
	popq     %r15
	popq     %r14
	popq     %r13
	popq     %r12
	ret      
10: 2

	.globl main
main:
	pushq    %r12
	pushq    %rbp
	movq     %rsp, %rbp
	subq     $80, %rsp
	leaq     -12(%rbp), %r10
	movq     %r10, -72(%rbp)
	movl     $0, -64(%rbp)
	movl     -64(%rbp), %r10d
	movslq   %r10d, %r10
	movq     %r10, -56(%rbp)
	movq     -56(%rbp), %r10
	imulq    $4, %r10
	movq     %r10, -48(%rbp)
	movq     -48(%rbp), %r10
	addq     -72(%rbp), %r10
	movq     %r10, -40(%rbp)
	movl     $1, -32(%rbp)
	movq     -40(%rbp), %r11
	movl     -32(%rbp), %r10d
	movl     %r10d, (%r11)
	leaq     -12(%rbp), %r10
	movq     %r10, -72(%rbp)
	movl     $1, -64(%rbp)
	movl     -64(%rbp), %r10d
	movslq   %r10d, %r10
	movq     %r10, -56(%rbp)
	movq     -56(%rbp), %r10
	imulq    $4, %r10
	movq     %r10, -48(%rbp)
	movq     -48(%rbp), %r10
	addq     -72(%rbp), %r10
	movq     %r10, -40(%rbp)
	movl     $2, -32(%rbp)
	movq     -40(%rbp), %r11
	movl     -32(%rbp), %r10d
	movl     %r10d, (%r11)
	leaq     -12(%rbp), %r10
	movq     %r10, -72(%rbp)
	movl     $2, -64(%rbp)
	movl     -64(%rbp), %r10d
	movslq   %r10d, %r10
	movq     %r10, -56(%rbp)
	movq     -56(%rbp), %r10
	imulq    $4, %r10
	movq     %r10, -48(%rbp)
	movq     -48(%rbp), %r10
	addq     -72(%rbp), %r10
	movq     %r10, -40(%rbp)
	movl     $3, -32(%rbp)
	movq     -40(%rbp), %r11
	movl     -32(%rbp), %r10d
	movl     %r10d, (%r11)
	leaq     -12(%rbp), %r10
	movq     %r10, -72(%rbp)
	movq     -72(%rbp), %rdi
	movl     $3, -64(%rbp)
	movl     -64(%rbp), %esi
	call     sum
	movl     %eax, %r12
	movl     %r12, %eax
	jmp      .Lmain_return
.Lmain_return:
	addq     $80, %rsp
	popq     %rbp
	popq     %r12
	ret      
