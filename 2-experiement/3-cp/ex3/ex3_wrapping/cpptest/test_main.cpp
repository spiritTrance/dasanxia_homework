int main(){
    int a = 9;
    int b = 3;
    return a + b;
}
// 最简单的能够执行的语句
// 	.globl	main
// main:
// 	li	a0,99
// 	jr	ra


// 	.file	"test.cpp"
// 	.option nopic
// 	.text
// 	.align	1
// 	.globl	main
// 	.type	main, @function
// main:
// .LFB0:
// 	.cfi_startproc
// 	addi	sp,sp,-16
// 	.cfi_def_cfa_offset 16
// 	sw	s0,12(sp)
// 	.cfi_offset 8, -4
// 	addi	s0,sp,16
// 	.cfi_def_cfa 8, 0
// 	li	a5,3
// 	mv	a0,a5
// 	lw	s0,12(sp)
// 	.cfi_restore 8
// 	.cfi_def_cfa 2, 16
// 	addi	sp,sp,16
// 	.cfi_def_cfa_offset 0
// 	jr	ra
// 	.cfi_endproc
// .LFE0:
// 	.size	main, .-main
// 	.ident	"GCC: (GNU) 9.2.0"
// 	.section	.note.GNU-stack,"",@progbits
