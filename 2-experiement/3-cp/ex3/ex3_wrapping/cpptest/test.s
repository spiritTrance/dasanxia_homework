	.file	"test.cpp"
	.option nopic
	.text
	.globl	arr
	.section	.sbss,"aw",@nobits
	.align	2
	.type	arr, @object
	.size	arr, 8
arr:
	.zero	8
	.text
	.align	1
	.globl	_Z6globalv
	.type	_Z6globalv, @function
_Z6globalv:
.LFB0:
	.cfi_startproc
	addi	sp,sp,-16
	.cfi_def_cfa_offset 16
	sw	s0,12(sp)
	.cfi_offset 8, -4
	addi	s0,sp,16
	.cfi_def_cfa 8, 0
	lui	a5,%hi(arr)
	addi	a5,a5,%lo(arr)
	li	a4,1
	sw	a4,0(a5)
	lui	a5,%hi(arr)
	addi	a5,a5,%lo(arr)
	li	a4,5
	sw	a4,4(a5)
	nop
	lw	s0,12(sp)
	.cfi_restore 8
	.cfi_def_cfa 2, 16
	addi	sp,sp,16
	.cfi_def_cfa_offset 0
	jr	ra
	.cfi_endproc
.LFE0:
	.size	_Z6globalv, .-_Z6globalv
	.align	1
	.globl	main
	.type	main, @function
main:
.LFB1:
	.cfi_startproc
	addi	sp,sp,-16
	.cfi_def_cfa_offset 16
	sw	ra,12(sp)
	sw	s0,8(sp)
	.cfi_offset 1, -4
	.cfi_offset 8, -8
	addi	s0,sp,16
	.cfi_def_cfa 8, 0
	lui	a5,%hi(arr)
	addi	a5,a5,%lo(arr)
	lw	a5,0(a5)
	mv	a0,a5
	call	_Z6putinti
	li	a0,10
	call	_Z5putchi
	lui	a5,%hi(arr)
	addi	a5,a5,%lo(arr)
	lw	a5,4(a5)
	mv	a0,a5
	call	_Z6putinti
	li	a0,10
	call	_Z5putchi
	li	a5,0
	mv	a0,a5
	lw	ra,12(sp)
	.cfi_restore 1
	lw	s0,8(sp)
	.cfi_restore 8
	.cfi_def_cfa 2, 16
	addi	sp,sp,16
	.cfi_def_cfa_offset 0
	jr	ra
	.cfi_endproc
.LFE1:
	.size	main, .-main
	.ident	"GCC: (GNU) 9.2.0"
	.section	.note.GNU-stack,"",@progbits
