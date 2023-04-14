```
enum  class  NodeType  {
        TERMINAL,              //  terminal  lexical  unit
        EXP,
        NUMBER,
        PRIMARYEXP,
        UNARYEXP,
        UNARYOP,
        MULEXP,
        ADDEXP,
        NONE
};
```

```
enum  class  TokenType{
        INTLTR,                //  int  literal
        PLUS,                //  +
        MINU,                //  -
        MULT,                //  *
        DIV,                //  /
        LPARENT,                //  (
        RPARENT,                //  )
};
```

# Grammar

```
Exp  ->  AddExp
        Exp.v
Number  ->  IntConst  |  floatConst
PrimaryExp  ->  '('  Exp  ')'  |  Number
        PrimaryExp.v
UnaryExp  ->  PrimaryExp  |  UnaryOp  UnaryExp
        UnaryExp.v
UnaryOp  ->  '+'  |  '-'  |  '!'
MulExp  ->  UnaryExp  {  ('*'  |  '/'  |  '%')  UnaryExp  }
        MulExp.v
AddExp  ->  MulExp  {  ('+'  |  '-')  MulExp  }
        AddExp.v
```

题目是四则运算，加减乘除括号，没有小数点貌似，但是有0x 0o 0b这种勾把玩意，处理的时候尤其注意，吐了。

看main函数，意思是有了语法解析树后要输出最终结果，注意到Node有value域，emm反正归约时肯定要改的，别慌。完全感觉可以波兰表达式偷鸡那种但是没必要。

看看这个文法又是什么东西，`*.v`这是啥意思？？？没看懂。

Token别忘了是一堆小数，麻花。

好好分析下这个文法，可以从优先级下手，首先那个大括号的意思个人感觉大概率是可选项，就是说 `AddExp  ->  MulExp  {  ('+'  |  '-')  MulExp  }`应该可以解释为 `AddExp  ->  MulExp  | MulExp '+' MulExp | MulExp '-' MulExp`

啊MotherFucker没看懂文法啊

[语法分析 · GitBook](http://114.117.246.238:4000/syntax.html)

没啥事了，我是消愁

继续看看，Number好说，

那PrimaryExp呢，看得出来他好像是在处理某个运算单元，这个单元可以是数字，也可以是括号括起来的结果？

看看UnaryExp是啥，看UnaryOp，应该是处理单纯一个数，或者前面加上正负号的各种数字，便于处理4*-9这种表达的？UnaryExp代表这种：`!9-8+7`这种？注意这里就要开始计算了，

然后UnaryExp出现在MulExp文法的右部。好说，有优先级。add完之后就变成Exp力！最后看得出来在哪里结束了吗？Exp。

好现在看看递归下降，首先不含左递归，然后看First集合，

emm 大括号的意思应该是零次或多次匹配= =
