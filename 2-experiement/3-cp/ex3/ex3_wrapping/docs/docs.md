# 实验三
## 实验目标
由实验二的 中间表示 IR 经过目标代码分析，生成可以与 sylib.a 链接的 risc-v 汇编

## 实验步骤
从希冀上下载实验框架
```bash
compiler [src_filename] -S -o [output_filename] 将输出你的汇编结果至 [output_filename]
```
**测评方法**：
使用 
```bash
riscv32-unknown-linux-gnu-gcc ur_assembly sylib-riscv-linux.a
```
 来编译你生成的 risc-v 汇编，生成 risc-v 的可执行文件

使用 qemu-riscv32 来模拟 risc-v 机器执行你的可执行文件

我们提供了脚本 qemu-riscv32.sh 来简化使用 qemu，你可以使用命令：qemu-riscv32.sh ur_rv_executable 来执行 risc-v 的可执行文件

## 实验三标准输出
这是一段简单的 SysY 程序
```cpp
int main() {
    return 3;
}
```
本实验没有标准答案，只要汇编可以被正确编译执行即可，gcc生成的汇编可以供你参考：

```cpp 
.file    "00_main.c"
    .option nopic
    .text
    .align    1
    .globl    main
    .type    main, @function
main:
    addi    sp,sp,-16
    sw    s0,12(sp)
    addi    s0,sp,16
    li    a5,3
    mv    a0,a5
    lw    s0,12(sp)
    addi    sp,sp,16
    jr    ra
    .size    main, .-main
    .ident    "GCC: (GNU) 9.2.0"
    .section    .note.GNU-stack,"",@progbits
```