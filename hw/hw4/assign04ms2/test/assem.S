0|struct Point|type|struct Point {}
1|x|variable|int
1|y|variable|int
0|main|function|function () returning int
2|p|variable|struct Point {int, int}
2|q|variable|pointer to struct Point {int, int}
	.section .text

	.globl main
main:
	pushq    %rbp
	movq     %rsp, %rbp
	subq     $64, %rsp
	leaq     -8(%rbp), %r10
	movq     %r10, -56(%rbp)
	movq     -56(%rbp), %r10
	movq     %r10, -64(%rbp)
	movq     $0, -56(%rbp)
	movq     -64(%rbp), %r10
	addq     -56(%rbp), %r10
	movq     %r10, -48(%rbp)
	movl     $2, -40(%rbp)
	movq     -48(%rbp), %r11
	movl     -40(%rbp), %r10d
	movl     %r10d, (%r11)
	movq     $4, -56(%rbp)
	movq     -64(%rbp), %r10
	addq     -56(%rbp), %r10
	movq     %r10, -48(%rbp)
	movl     $3, -40(%rbp)
	movq     -48(%rbp), %r11
	movl     -40(%rbp), %r10d
	movl     %r10d, (%r11)
	movq     $0, -56(%rbp)
	movq     -64(%rbp), %r10
	addq     -56(%rbp), %r10
	movq     %r10, -48(%rbp)
	movq     $4, -40(%rbp)
	movq     -64(%rbp), %r10
	addq     -40(%rbp), %r10
	movq     %r10, -32(%rbp)
	movq     -48(%rbp), %r11
	movq     -32(%rbp), %r11
	movl     (%r11), %r10d
	addl     (%r11), %r10d
	movl     %r10d, -24(%rbp)
	movl     -24(%rbp), %eax
	jmp      .Lmain_return
.Lmain_return:
	addq     $64, %rsp
	popq     %rbp
	ret      
