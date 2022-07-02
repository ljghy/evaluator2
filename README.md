# Expression evaluator in C++17

## Build and run

```
mkdir build
cd build
```

#### Windows MSVC

```
cmake ..
cmake --build . --config [Debug|Release]
..\bin\eval.exe
```

#### Windows MinGW

```
cmake -G"MinGW Makefiles" -DCMAKE_BUILD_TYPE=[Debug|Release] ..
mingw32-make
..\bin\eval.exe
```

#### Linux GCC

```
cmake -DCMAKE_BUILD_TYPE=[Debug|Release] ..
make 
../bin/eval
```

## Examples

### Arithmatic Expressions
```
sin(pi / 2) + sqrt(2) ^ gamma(3) - 4 * 5 / sin(-6 - 7 / ln(8)) - 9
```
### List 
```
[]
[1, 2, 3, 4, 5]
[1 + 2, (3 - 4) / 5, 6]
```

### Lambda
```
@(x){x^2}
@(x, y){x + y}
@(x, y, z){[x + y, y + z, z + x]}
@(a){@(x){x + a}}
```

### List Arithmatic Expressions
```
1 + [2, 3]
[1, 2] + [3, 4]
[3, 4] * 5
[3, 4] / [5, 6]
([1, 2] - 1.5)^2
sin([1, 2, 3, 4])[1]
len([1, 2, 3, 4])
```

### Mixed
```
f(x, y) = sqrt(x^2 + y^2)
f(3, 4)
fib(n) = if_else(gt(n, 1), fib(n - 1) + fib(n - 2), 1)

sum_list(l, n) = if_else(gt(n, 1), sum_list(l, n - 1) + l[n - 1], l[0])
sum = @(l){sum_list(l, len(l))}
sum([1, 2, 3])
mean(l) = sum(l) / len(l)

Y = @(f){@(g){f(g(g))}(@(g){f(@(y){g(g)(y)})})}
fact_gen = @(f){@(n){if_else(n, n * f(n - 1), 1)}}
fact = Y(fact_gen)
fact(5)
```

### Commands
```
!init
!list
!exit
```
## Specification

### EBNF
```
ASSIGN -> ident '=' EXPR
ASSIGN_LABMDA -> ident '(' PARAM_LIST ')' '=' EXPR
EXPR -> EXPR_L1

TERM -> decimal | ident | '(' EXPR ')' | LIST | LAMBDA | INDEX | CALL 
LIST -> '[' EXPR_LIST ']'
LAMBDA -> '@' '(' PARAM_LIST ')' '{' EXPR '}'
INDEX -> TERM '[' EXPR ']'
CALL -> TERM '(' EXPR_LIST ')'
PARAM_LIST -> empty | ident {',' ident}
EXPR_LIST -> empty | EXPR {',' EXPR}
EXPR_UNARY -> ['-'] EXPR_L2
EXPR_L1 -> EXPR_UNARY {OPTR_L1 EXPR_UNARY}
EXPR_L2 -> EXPR_L3 {OPTR_L2 EXPR_L3}
EXPR_L3 -> TERM {'^' TERM}
OPTR_L1 -> '+' | '-'
OPTR_L2 -> '*' | '/'
```

### Internal Variables
```
pi
e
ans
```

### Internal Functions
```
sqrt(x)
sin(x)
cos(x)
tan(x)
asin(x)
acos(x)
atan(x)
exp(x)
ln(x)
abs(x)
round(x)
floor(x)
ceil(x)
gamma(x)
erf(x)

len(list)

not(x)
and(x, y)
or(x, y)

eq(x, y)
neq(x, y)
gt(x, y)
lt(x, y)
geq(x, y)
leq(x, y)

if_else(cond, true, false)
```