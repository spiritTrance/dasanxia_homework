	.file	"test.cpp"
	.option nopic
	.text
	.globl	b
	.section	.sdata,"aw"
	.align	2
	.type	b, @object
	.size	b, 4
b:
	.word	9
	.text
	.align	1
	.globl	main
	.type	main, @function
main:
.LFB0:
	.cfi_startproc
	addi	sp,sp,-32
	.cfi_def_cfa_offset 32
	sw	ra,28(sp)
	sw	s0,24(sp)
	.cfi_offset 1, -4
	.cfi_offset 8, -8
	addi	s0,sp,32
	.cfi_def_cfa 8, 0
	li	a5,9
	sw	a5,-20(s0)
	lw	a4,-20(s0)
	li	a5,8
	bne	a4,a5,.L2
	li	a5,8
	sw	a5,-20(s0)
	j	.L3
.L2:
	li	a5,6
	sw	a5,-20(s0)
.L3:
	li	a5,1
	sw	a5,-24(s0)
.L7:
	lw	a4,-24(s0)
	li	a5,100
	bgt	a4,a5,.L4
	lw	a5,-24(s0)
	sw	a5,-28(s0)
.L6:
	lw	a5,-28(s0)
	blt	a5,zero,.L5
	lw	a4,-24(s0)
	lw	a5,-28(s0)
	mul	a4,a4,a5
	lw	a3,-20(s0)
	lw	a5,-28(s0)
	div	a5,a3,a5
	add	a5,a4,a5
	sw	a5,-32(s0)
	lw	a5,-28(s0)
	addi	a5,a5,-1
	sw	a5,-28(s0)
	j	.L6
.L5:
	lw	a5,-24(s0)
	addi	a5,a5,1
	sw	a5,-24(s0)
	j	.L7
.L4:
	lw	a0,-20(s0)
	call	_Z6putinti
	lw	a5,-20(s0)
	mv	a0,a5
	lw	ra,28(sp)
	.cfi_restore 1
	lw	s0,24(sp)
	.cfi_restore 8
	.cfi_def_cfa 2, 32
	addi	sp,sp,32
	.cfi_def_cfa_offset 0
	jr	ra
	.cfi_endproc
.LFE0:
	.size	main, .-main
	.ident	"GCC: (GNU) 9.2.0"
	.section	.note.GNU-stack,"",@progbits
