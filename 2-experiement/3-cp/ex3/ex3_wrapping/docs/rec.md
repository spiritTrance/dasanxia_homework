wsl
```bash
sudo service docker start;
# docker run -it -v /mnt/c/Users/15781/Desktop/dasanxia_homework/2-experiement/3-cp/ex3/ex3_wrapping:/coursegrader --name cpEx frankd35/demo:v3 
docker start cpEx;
docker attach cpEx;
```

```bash
cd ../cppTest; riscv32-unknown-linux-gnu-gcc test.cpp -S
riscv32-unknown-linux-gnu-gcc test.s ../test/sylib-riscv-linux.a -o test.exe
qemu-riscv32.sh test.exe
cd ../cppTest; riscv32-unknown-linux-gnu-gcc test.cpp -S; riscv32-unknown-linux-gnu-gcc test.s ../test/sylib-riscv-linux.a -o test.exe; qemu-riscv32.sh test.exe; echo $?
cd ../cppTest; riscv32-unknown-linux-gnu-gcc test.s ../test/sylib-riscv-linux.a -o test.exe; qemu-riscv32.sh test.exe; echo $?
```


```bash
cd ../bin; ./compiler ../test/testcase/basic/03_arr_defn2.sy -s2 -o test.txt
cd ../bin; ./compiler ../test/testcase/basic/03_arr_defn2.sy -e -o _test.txt
cd ../test; python3 run.py s2; python3 score.py s2
cd ../build; make; cd ../bin
cd ../test; python3 score.py s2
cd ../build; make; cd ../test; python3 run.py s2; python3 score.py s2
cd ../bin; ./compiler ../test/testcase/function/79_var_name.sy -s2 -o test.txt
cd ../bin; ./compiler ../test/testcase/function/79_var_name.sy -e -o test_.txt
```



修改命令行颜色
```bash
cd ~;
gedit .bashrc;
vim ~/.bashrc;
source ~/.bashrc;
PS1='\[\e[1;35m\]\u@\h:\[\e[0m\]\[\e[1;33m\]\w\[\e[1;35m\]\[\e[0m\]\[\e[1;34m\]\$\[\e[0m\]'
PS1='\[\e[1;31m\]\u@\h:\[\e[0m\]\[\e[1;33m\]\w\[\e[1;35m\]\[\e[0m\]\[\e[1;34m\]\$\[\e[0m\]'
PS1="\[\033[1;36;01m\]\u\[\033[00m\]\[\033[1;34;01m\]@\[\033[00m\]\[\033[1;32;01m\]\h\[\033[00m\]\[\033[34;01m\]:\[\033[00m\]\[\033[33;01m\]\w\[\033[00m\]\[\033[31;01m\] \$\[\033[37;00m\] "
PS1="\[\033[34;01m\][\[\033[1;34;01m\]\u\[\033[00m\]\[\033[1;32;01m\]@\[\033[00m\]\[\033[1;34;01m\]\h\[\033[00m\]\[\033[34;01m\]] \[\033[00m\]\[\033[33;01m\]\w\[\033[00m\]\[\033[35;01m\] \n>>>\[\033[37;00m\] "
PS1="\[\033[00m\]\[\033[33;01m\]\w\[\033[00m\]\[\033[35;01m\] >>>\[\033[37;00m\] "
PS1="\[\033[35;01m\] >>>\[\033[37;00m\] "
```
常用的颜色为 红色31、绿色32、黄色33、蓝色34、紫红色35、青蓝色36、白色37。颜色右边的“01m"代表字体是否加粗。更详细的内容可以进第一个参考资料查看。
(在hacker dark theme下)
紫色30 棕色31 红色32 绿色33 黄色34 绿色35 灰色36,38,24-28 白色37 棕色反底41 红色反底42 绿色反底43 黄色反底44
(原配主题下)
红色31绿色32黄色33s深蓝色34紫色35蓝色36）
