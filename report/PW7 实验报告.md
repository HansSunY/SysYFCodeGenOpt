# PW7 实验报告

## 必做

### Part1-公共子表达式删除

#### 样例

```c
/*cse的测试*/
//块内
int f1(){
	int a=10;
	int b=15;
	int c=1;
	int d=(b*c)*12+a+(a+b*c);
	int e[1]={1};
	a=e[b*c];
}
//块间
int f2(){
	int a=10;
	int b=10;
	int d=a+b;//a+b
	int c=0;
	if(a>0){
		c=a+b;//a+b
	}
	d=c-a;
}
//phi
int f3(int m,int n){
        int a,b,c,d,e;
        if(m){
            a = m*n;
            b = m-n;
        }
        else{
            a = m-n;
            b = m*n;
        }
        d = m*n;
        e = m-n;
        return d+e;
    }

```

#### 结果

对f1函数，b*c为公共子表达式，需要将其消除

```
define i32 @f1() {
label_entry:
  %op7 = mul i32 15, 1
  %op8 = mul i32 %op7, 12
  %op10 = add i32 %op8, 10
  %op15 = add i32 10, %op7
  %op16 = add i32 %op10, %op15
  %op17 = alloca [1 x i32]
  %op18 = getelementptr [1 x i32], [1 x i32]* %op17, i32 0, i32 0
  store i32 1, i32* %op18
  %op22 = getelementptr [1 x i32], [1 x i32]* %op17, i32 0, i32 %op7
  %op23 = load i32, i32* %op22
  br label %label_ret
label_ret:                                             ; preds = %label_entry
  ret i32 0
}
```

对f2函数，a+b为公共子表达式且位于不同块中，label_entry时label13的支配结点，需要全局上的消除

```
define i32 @f2() {
label_entry:
  %op6 = add i32 10, 10
  %op9 = icmp sgt i32 10, 0
  %op10 = zext i1 %op9 to i32
  %op11 = icmp ne i32 %op10, 0
  br i1 %op11, label %label13, label %label17
label_ret:                                           ; preds = %label17
  ret i32 0
label13:                                             ; preds = %label_entry
  br label %label17
label17:                                             ; preds = %label_entry, %label13
  %op21 = phi i32 [ 0, %label_entry ], [ %op6, %label13 ]
  %op20 = sub i32 %op21, 10
  br label %label_ret
}
```

对f3函数，对于多个前驱块，需要添加phi指令。

```
define i32 @f3(i32 %arg0, i32 %arg1) {
label_entry:
  %op11 = icmp ne i32 %arg0, 0
  br i1 %op11, label %label13, label %label20
label_ret:                                                ; preds = %label27
  ret i32 %op36
label13:                                                ; preds = %label_entry
  %op16 = mul i32 %arg0, %arg1
  %op19 = sub i32 %arg0, %arg1
  br label %label27
label20:                                                ; preds = %label_entry
  %op23 = sub i32 %arg0, %arg1
  %op26 = mul i32 %arg0, %arg1
  br label %label27
label27:                                                ; preds = %label13, %label20
  %op37 = phi i32 [ %op19, %label13 ], [ %op23, %label20 ]
  %op38 = phi i32 [ %op16, %label13 ], [ %op26, %label20 ]
  %op36 = add i32 %op38, %op37
  br label %label_ret
}
```



#### B1-1

​	由于实验提供的CFG为SSA形式，故可用表达式分析可基本略去，我们采用比较直接的方式来进行公共表达式删除。

​	首先，用is_valid_expr函数筛选出需要处理的指令，对这些指令进行逐一比对，如果其对应操作数相同，则进行如下操作：

1. 判断两条指令之间的关系，若两条指令位于同一基本块中，则删除较后出现的指令；若两条指令所在基本块间存在支配关系，则删除被支配基本块中的指令；其他情况不处理,确保不带来副作用。
2. 将被删除指令的引用全部更改为其公共表达式，完成公共表达式删除。

#### B1-2

在call指令和load指令中，考虑操作数为指针时其指向值可能改变，这需要改变公共表达式的判断条件。



### Part2-活跃变量分析

#### 样例

``` C
int main(){
    int a = 3;
    if(b>c){
        a = a + 2;
        c = c + 2;
    }else{
        a = a - 2;
        c = a - 2;
    }
    b = a;
    return b;
}
```

```
define i32 @main() {
label_entry:
  %op4 = add i32 3, 1
  %op7 = add i32 3, 1
  %op10 = icmp sgt i32 %op4, %op7
  %op11 = zext i1 %op10 to i32
  %op12 = icmp ne i32 %op11, 0
  br i1 %op12, label %label14, label %label19
label_ret:                                                ; preds = %label24
  ret i32 %op28
label14:                                                ; preds = %label_entry
  %op16 = add i32 3, 2
  %op18 = add i32 %op7, 2
  br label %label24
label19:                                                ; preds = %label_entry
  %op21 = sub i32 3, 2
  %op23 = sub i32 %op21, 2
  br label %label24
label24:                                                ; preds = %label14, %label19
  %op27 = phi i32 [ %op23, %label19 ], [ %op18, %label14 ]
  %op28 = phi i32 [ %op21, %label19 ], [ %op16, %label14 ]
  br label %label_ret
}
```

以下为活跃变量输出结果，经计算对比与上述计算结果一致

```
label_entry:
  in = {op4, op7, op10, op11, op12, op21, }
  out = {op7, op21, }

label_ret:
  in = {op28, }
  out = {}

label14:
  in = {op7, }
  out = {op16, op18, }

label19:
  in = {op21, }
  out = {op21, op23, }

label24:
  in = {}
  out = {op28, }
```

#### B2-1

在本次实验中使用了去除了load和store指令的**SSA**形式的CFG，在生成的中间代码中使用了phi语句，在进行活跃变量分析时，需要对phi语句进行不同的处理方式。

以下为本次实验中使用的活跃变量分析算法伪代码：

``` C
// 首先将所有集合置空
for each bb in func_:
    bb.live_in=bb.live_out={};
// 对函数中的每一个变量，遍历每一个基本块，如果基本块内使用了该变量，
// 调用set_live()函数,此处的has_used()函数不会检测含有phi语句的情况，
// has_phi_used()函数对在phi语句内使用的变量进行单独处理
for each value in func_:
    for each bb in func_:
        if(has_used(value,bb)):
            set_live(value,bb);
        has_phi_used(value,bb);

// set_live()函数
set_live(value,bb):
    // 将value加入到bb.live_in
    bb.live_in.insert(value);
    // 遍历bb的每一个前驱，并将value加入到前驱块的live_out中，
    // 若value未在前驱块中定值，则调用set_live(value,pre_bb)到前驱块寻找
    for each pre_bb in bb.pre_bbs:
        pre_bb.live_out.insert(value);
        if(!has_defined(value,pre_bb))
            set_live(value,pre_bb);

// has_phi_used()函数
has_phi_used(value,bb):
    // 遍历bb中的每一个指令，如果是含有value的phi指令，则将value加入到其对应
    // 前驱块的live_out中
    for each inst in bb.instructions:
        if inst.type == phi:
            for each [value,pre_bb] in phi:
                pre_bb.live_out.insert(value);
```

对于phi语句内的变量的处理，将其视作在其来源基本块的out处为活跃变量，在phi语句的in处不是活跃变量。即将该变量的活跃性视作在语句块传递的途中结束。



### Part3-支配树

#### B3-1

证明：由$x,y$支配$b$，则从起点到达$b$的每条路径都要经过$x$和$y$。反证，不失一般性，设$x$不支配$y$，则至少存在一条到达$y$的路径不经过$x$，这样取一条从$y$到$b$且不包含$x$的路径即可得到一条从起点开始，经过$y$但不经过$x$的路径，矛盾。

#### B3-2

论文中有描述**Thus, an iterative algorithm that traverses the graph in reverse postorder will halt in no more than d(G) + 3 passes, where d(G) is the loop connectedness of the grap**，所以可能是只有使用后序遍历的逆序才能达到论文中所说的复杂度。而无论何种顺序应该不影响算法正确性。

#### B3-3

由论文知方程为
$$
Dom(b) = {b} \cup IDom(b) \cup IDom(IDom(b))\cdots \{n0\}
$$
这种迭代当求值到后序遍历序的前面的节点时，不改变前面的值，但会改变后面的值，所以要按后序的逆序，即从前往后进行。

#### B3-4

intersect的作用是找到支配节点的交集。

不能改变，因为**nodes higher in the dominator tree have higher postorder numbers**。 

#### B3-5

支配树：the total cost per iteration is $O(N + E\cdot D)$ where D is the size of the largest Dom，而迭代次数不大于$d(G) +3$。空间：$O(n^2)$，即支配树

教材：时空复杂度都是$O(n^3)$

复杂度对于教材都有优势

#### B3-6

将ret语句视为exit节点。

因为可能最后一个基本块属于某个循环，无法区分出最后一个基本块的确切位置。



## 选做

### 循环不变式外提

#### 样例

```c
/*循环不变式外提的测试*/
//两层循环，只需外提一层的情况(a+b)
int f1() {
	int a=10;
	int b=10;
	int c=0;
	int d=100;
	int e=10;
	int f=10;
	int h;
	while(b>0){
		a=a-c;
		b=b-c;
		h=e+f;//外提至最外层
		while(a>0){
			c=a+b;//外提至上层循环
			d=d-c;
		}
	}
 }
//while条件内
int f2(){
	int a=10;
	int b=10;
	int c=0;
	int d=100;
	if(a<10){
		a=a-1;
	}
	while(a-b){
		c=a+b;
		d=d-c;
	}
}
//嵌套外提，a+b外提后c-a也满足外提条件
int f3(){
	int a=10;
	int b=10;
	int c=0;
	int d=100;
	while(a-b){
		c=a+b;
		d=c-a;
	}
}
```

#### 结果

对于f1函数第一层循环中的h=e+f被提到最外层，第二层循环中的c=a+b被提到第一层循环中

```
define i32 @f1() {
label_entry:
  %op23 = add i32 10, 10				   ; h=e+f
  br label %label9
label_ret:                                 ; preds = %label24
  ret i32 0
label9:                                    ; preds = %label_entry, %label37
  %op38 = phi i32 [ 100, %label_entry ], [ %op42, %label37 ]
  %op39 = phi i32 [ 0, %label_entry ], [ %op43, %label37 ]
  %op40 = phi i32 [ 10, %label_entry ], [ %op20, %label37 ]
  %op41 = phi i32 [ 10, %label_entry ], [ %op17, %label37 ]
  %op11 = icmp sgt i32 %op40, 0
  %op12 = zext i1 %op11 to i32
  %op13 = icmp ne i32 %op12, 0
  br i1 %op13, label %label14, label %label24
label14:                                   ; preds = %label9
  %op17 = sub i32 %op41, %op39
  %op20 = sub i32 %op40, %op39
  %op33 = add i32 %op17, %op20			   ; c=a+b
  br label %label25
label24:                                   ; preds = %label9
  br label %label_ret
label25:                                   ; preds = %label14, %label30
  %op42 = phi i32 [ %op38, %label14 ], [ %op36, %label30 ]
  %op43 = phi i32 [ %op39, %label14 ], [ %op33, %label30 ]
  %op27 = icmp sgt i32 %op17, 0
  %op28 = zext i1 %op27 to i32
  %op29 = icmp ne i32 %op28, 0
  br i1 %op29, label %label30, label %label37
label30:                                   ; preds = %label25
  %op36 = sub i32 %op42, %op33
  br label %label25
label37:                                   ; preds = %label25
  br label %label9
}
```

对于f2函数，while条件表达式a-b和循环体中的c=a+b都需要外提

```
define i32 @f2() {
label_entry:
  %op6 = icmp slt i32 10, 10
  %op7 = zext i1 %op6 to i32
  %op8 = icmp ne i32 %op7, 0
  br i1 %op8, label %label10, label %label13
label_ret:                                  ; preds = %label26
  ret i32 0
label10:                                    ; preds = %label_entry
  %op12 = sub i32 10, 1
  br label %label13
label13:                                    ; preds = %label_entry, %label10
  %op27 = phi i32 [ 10, %label_entry ], [ %op12, %label10 ]
  %op22 = add i32 %op27, 10					; c=a+b
  %op17 = sub i32 %op27, 10					; a-b
  br label %label14
label14:                                    ; preds = %label13, %label19
  %op28 = phi i32 [ 100, %label13 ], [ %op25, %label19 ]
  %op18 = icmp ne i32 %op17, 0
  br i1 %op18, label %label19, label %label26
label19:                                    ; preds = %label14
  %op25 = sub i32 %op28, %op22
  br label %label14
label26:                                    ; preds = %label14
  br label %label_ret
}
```

对于f3函数，首先将c=a+b外提，c的定值外提后d=c-a也可继续外提

```
define i32 @f3() {
label_entry:
  %op14 = add i32 10, 10                    ; c=a+b
  %op17 = sub i32 %op14, 10					; d=c-a
  %op9 = sub i32 10, 10
  br label %label6
label_ret:                                  ; preds = %label18
  ret i32 0
label6:                                     ; preds = %label_entry, %label11
  %op10 = icmp ne i32 %op9, 0
  br i1 %op10, label %label11, label %label18
label11:                                    ; preds = %label6
  br label %label6
label18:                                    ; preds = %label6
  br label %label_ret
}

```

#### 设计思路

设计思路如下：

1. 使用Tarjan算法找出CFG中的极大连通分量
2. 当中间代码结构改变时，循环进行循环不变式的外提操作，直至中间代码无变化
3. 对循环进行递归逐层处理，先处理循环中的小循环，逐层外提
4. 若表达式的操作数为常数或未在当前循环中定义，则判断为不变式。考虑到其所在标准块可能有多个前驱，则统一将不变式外提到其所在基本块所有前驱块的最近共同支配结点。



### 常量传播

#### 样例

``` C
int f1(){
    int a = 3 + 3;
    int b = a * 12 / 4 + a;
    int c = b;
    int f[4] = {1,2,3,4};
    int g = f[0]*3+2;
    float fa = 3.19 * 3.14;
    float fb = fa * fa + 3.1415926 / a;
    int t = fb + 3;
    return c;
}
```

从结果可以看出，对所有的常量进行传播，并对后续不再使用的无效代码进行了删除，数组变量没有进行优化，所有可以进行传播的常量均进行了优化，使得在后续中没有使用的常量代码均进行了删除

```
define i32 @f1() {
label_entry:
  %op10 = alloca [4 x i32]
  %op11 = getelementptr [4 x i32], [4 x i32]* %op10, i32 0, i32 0
  store i32 1, i32* %op11
  %op12 = getelementptr [4 x i32], [4 x i32]* %op10, i32 0, i32 1
  store i32 2, i32* %op12
  %op13 = getelementptr [4 x i32], [4 x i32]* %op10, i32 0, i32 2
  store i32 3, i32* %op13
  %op14 = getelementptr [4 x i32], [4 x i32]* %op10, i32 0, i32 3
  store i32 4, i32* %op14
  %op16 = getelementptr [4 x i32], [4 x i32]* %op10, i32 0, i32 0
  %op17 = load i32, i32* %op16
  %op18 = mul i32 %op17, 3
  %op19 = add i32 %op18, 2
  br label %label_ret
label_ret:                                                ; preds = %label_entry
  ret i32 24
}
```

``` C
int f2(){
    if(3 > 2 && 4 < 3 || 6 >= 0){
        if( 6 > 3 && 4 <= 9)
            return 3;
        else 
            return 16;
    }else{
        while(1 > 2 || 3 < 4){
            return 4;
        }
    }
}
```

以下是常量传播优化后的代码

```
define i32 @f2() {
label_entry:
  br label %label15
label_ret:                                                ; preds = %label19
  ret i32 3
label5:                                                ; preds = %label11
  br label %label21
label11:                                                ; preds = %label15
  br label %label5
label15:                                                ; preds = %label_entry
  br label %label11
label19:                                                ; preds = %label21
  br label %label_ret
label21:                                                ; preds = %label5
  br label %label19
}
```

以下是未进行常量传播优化的代码

```
define i32 @f2() {
label_entry:
  %op1 = icmp sgt i32 3, 2
  %op2 = zext i1 %op1 to i32
  %op3 = icmp ne i32 %op2, 0
  br i1 %op3, label %label15, label %label11
label_ret:                                                ; preds = %label19, %label20, %label29, %label10
  %op35 = phi i32 [ 0, %label10 ], [ 4, %label29 ], [ 16, %label20 ], [ 3, %label19 ]
  ret i32 %op35
label5:                                                ; preds = %label15, %label11
  %op6 = icmp sgt i32 6, 3
  %op7 = zext i1 %op6 to i32
  %op8 = icmp ne i32 %op7, 0
  br i1 %op8, label %label21, label %label20
label9:                                                ; preds = %label11
  br label %label25
label10:                                                ; preds = %label30
  br label %label_ret
label11:                                                ; preds = %label_entry, %label15
  %op12 = icmp sge i32 6, 0
  %op13 = zext i1 %op12 to i32
  %op14 = icmp ne i32 %op13, 0
  br i1 %op14, label %label5, label %label9
label15:                                                ; preds = %label_entry
  %op16 = icmp slt i32 4, 3
  %op17 = zext i1 %op16 to i32
  %op18 = icmp ne i32 %op17, 0
  br i1 %op18, label %label5, label %label11
label19:                                                ; preds = %label21
  br label %label_ret
label20:                                                ; preds = %label5, %label21
  br label %label_ret
label21:                                                ; preds = %label5
  %op22 = icmp sle i32 4, 9
  %op23 = zext i1 %op22 to i32
  %op24 = icmp ne i32 %op23, 0
  br i1 %op24, label %label19, label %label20
label25:                                                ; preds = %label9
  %op26 = icmp sgt i32 1, 2
  %op27 = zext i1 %op26 to i32
  %op28 = icmp ne i32 %op27, 0
  br i1 %op28, label %label29, label %label31
label29:                                                ; preds = %label25, %label31
  br label %label_ret
label30:                                                ; preds = %label31
  br label %label10
label31:                                                ; preds = %label25
  %op32 = icmp slt i32 3, 4
  %op33 = zext i1 %op32 to i32
  %op34 = icmp ne i32 %op33, 0
  br i1 %op34, label %label29, label %label30
}
```

对比后可以看出，优化后的代码部分将所有的无效语句块全部删除，只留下了有使用的路径，并且常量传播过程对于cmp、zext指令同样有效，从而使得即使是嵌套的无效语句块也能被删除，在这个过程中，对于语句块删除后的phi指令同样进行了调整，减少了相应的分支，对于只有一条确定分支的phi指令同样进行常量传播处理，直至代码优化到无法继续常量传播。

#### 设计思路

1. 遍历每一个函数，并依次进行常量传播、无效条件分支的删除、phi语句的处理三个步骤。
2. 常量传播过程：遍历每一条指令，判断其操作数是否均为常量，若均为常量则计算出该语句的结果并暂存，将代码中所有使用到该变量的地方用计算出的常量替代。之后删除该条指令。
3. 无效条件分支删除：遍历每一条指令，判断其是否为条件跳转指令，若是，则判断其条件是否为常量，若为常量则将无效语句块与当前语句块的连接切断，并将条件跳转指令删除，插入新的跳转指令。(1)结束上述过程后，重新遍历基本块，若其无前驱块并且非起始块则将其与后驱块的连接断开，然后将该块删除。重复(1)过程直至无可删除的基本块。
4. phi指令处理：遍历每一个基本块内的指令，若为phi指令，将phi指令中已经不存在的语句块对应部分删除。
5. 重复1过程，直至4过程内无变化。
