# Project Assignment 5 - Code Generation

**Introduction to Compiler Design by Prof. Yi-Ping You**

Due Date: **23:59, January 3, 2025**

**Late submission is not allowed**

Your assignment is to generate `RISC-V instructions` for the **`P`** language based on the `AST` and `symbol table` that you have built in the previous assignments. The generated code will then be executed on a RISC-V simulator, called `Spike`.

### The description video of homework:

https://drive.google.com/drive/folders/1bxhnd03dAB2DbRrAsqgB13DjRHQMJVLW

---

## Table of Contents

- [Project Assignment 5 - Code Generation](#project-assignment-5---code-generation)
  - [Table of Contents](#table-of-contents)
  - [Assignment](#assignment)
  - [Generating RISC-V Instructions](#generating-risc-v-instructions)
    - [Initialization](#initialization)
    - [Declarations of Variables and Constants](#declarations-of-variables-and-constants)
      - [Global Variables](#global-variables)
      - [Local Variables](#local-variables)
      - [Global Constants](#global-constants)
      - [Local Constants](#local-constants)
    - [Expression](#expression)
    - [Function Declaration and Invocation](#function-declaration-and-invocation)
    - [Simple Statement](#simple-statement)
    - [Conditional Statement](#conditional-statement)
    - [For Statement and While Statement](#for-statement-and-while-statement)
    - [Combining All Examples Above](#combining-all-examples-above)
    - [Bonus](#bonus)
      - [Boolean Type](#boolean-type)
      - [Array Type](#array-type)
      - [String Type](#string-type)
      - [Real Type](#real-type)
  - [Implementation Hints](#implementation-hints)
    - [Using `C` compiler that targets RISC-V](#using-c-compiler-that-targets-risc-v)
  - [Project Structure](#project-structure)
  - [Assessment Rubrics (Grading)](#assessment-rubrics-grading)
  - [Build and Execute](#build-and-execute)
    - [Build Project](#build-project)
    - [Test your compiler with the simulator](#test-your-compiler-with-the-simulator)
    - [Simulator Commands](#simulator-commands)
    - [Test your compiler with the RISC-V development board](#test-your-compiler-with-the-risc-v-development-board)
  - [Submitting the Assignment](#submitting-the-assignment)

---

## Assignment

In order to keep this assignment simple, only the `integer` type is needed to be implemented and the `array` type is not considered. Your assignment is to generate `RISC-V` instructions for a `P` program that contains any of the following constructs:

- Global variable or local variable declaration.
- Global constant or local constant declaration.
- Function declaration.
- Assign statement.
- Simple statement.
- Expressions with only `+` `-` (unary and binary) `*` `/` `mod` `function invocation` included.
- Function invocation statement.
- Conditional statement.
- For statement and while statement.

The generated `RISC-V` instructions should be saved in a file with the same name as the input `P` file but with a `.S` extension. In addition, the file should be stored in a directory, which is set by the flag `--save-path [save path]`. For example, the following command translates `./test.p` into `../test/riscv/test.S`.

```sh
./compiler test.p --save-path ../test/riscv
```

<div align="right">
    <b><a href="#table-of-contents">↥ back to menu</a></b>
</div>

## Generating RISC-V Instructions

> In the following subsections, we provide some examples of how to translate a `P` construct into RISC-V instructions. You may design your own instruction selection rules, as long as the generated code does what it should do. We recommend you read the [`RISC-V` tutorial](RISC-V-tutorial) before getting into the following subsections.

For all the examples below, we use a simple computation model called [**stack machine**](https://en.wikipedia.org/wiki/Stack_machine).

- When traversing to a `variable reference` node, push the **value** of the variable on the stack if it appears on the `RHS` of the `assignment` node, or push the **address** of the variable to the stack if it's on the `LHS` of the `assignment` node.

- When traversing to a `computation` node, (1) pop the values on the stack to some registers, and (2) then compute the result with one or more instructions, and (3) finally push the result back to the stack.

- When traversing to an `assignment` node, (1) pop the value and the address on the stack to some registers, and (2) then store the value to that address.

- For more precise steps, see [simple compilers](https://en.wikipedia.org/wiki/Stack_machine#Simple_compilers).

---

The generated `RISC-V` code will have the following structure:

```assembly
.section    .bss
    # uninitialized global variable(s)

.section    .rodata
    # global constant(s)

.section    .text
    # function

.section    .text
    # main function
```

<div align="right">
    <b><a href="#table-of-contents">↥ back to menu</a></b>
</div>

### Initialization

- An empty `P` program

  ```pascal
  // test1.p
  test1;
  begin
  end
  end
  ```

  will be translated into the following `RISC-V` instructions.

  ```assembly
      .file "test1.p"
      .option nopic
  .section    .text
      .align 2
      .globl main          # emit symbol 'main' to the global symbol table
      .type main, @function
  main:
      # in the function prologue
      addi sp, sp, -128    # move stack pointer to lower address to allocate a new stack
      sw ra, 124(sp)       # save return address of the caller function in the current stack
      sw s0, 120(sp)       # save frame pointer of the last stack in the current stack
      addi s0, sp, 128     # move frame pointer to the bottom of the current stack

      # in the function epilogue
      lw ra, 124(sp)       # load return address saved in the current stack
      lw s0, 120(sp)       # move frame pointer back to the bottom of the last stack
      addi sp, sp, 128     # move stack pointer back to the top of the last stack
      jr ra                # jump back to the caller function
      .size main, .-main
  ```

A function `main` must be generated for the compound statement (program body) in the program node.

You should allocate a local memory in the function prologue and clear the local memory in the function epilogue. In this assignment, allocate **128 bytes** of local memories is sufficient for the parameters and the local variables. However, in modern compilers, the size of the local memory depends on the demand of the function.

<div align="right">
    <b><a href="#table-of-contents">↥ back to menu</a></b>
</div>

### Declarations of Variables and Constants

#### Global Variables

- Declaring a global variable `a`

  ```pascal
  var a: integer;
  ```

  will be translated into the following `RISC-V` instructions.

  ```assembly
  .comm a, 4, 4    # emit object 'a' to .bss section with size = 4, align = 4
  ```

- Assigning a value to a global variable `a`

  ```pascal
  a := 6;
  ```

  will be translated into the following `RISC-V` instructions.

  ```assembly
  la t0, a         # load the address of variable 'a'
  addi sp, sp, -4
  sw t0, 0(sp)     # push the address to the stack
  li t0, 6         # load value '6' to register 't0'
  addi sp, sp, -4
  sw t0, 0(sp)     # push the value to the stack
  lw t0, 0(sp)     # pop the value from the stack
  addi sp, sp, 4
  lw t1, 0(sp)     # pop the address from the stack
  addi sp, sp, 4
  sw t0, 0(t1)     # save the value to the address of 'a'
  ```

#### Local Variables

- Declaring a local variable

  - Each local integer variable occupies four bytes of the allocated local memory. For example, `fp-8` to `fp-12` for variable `b` and `fp-12` to `fp-16` for variable `c`. You could save this information in the `symbol table` the first time you construct it.

- Assigning a value to a local variable `b` stored in `fp-8` to `fp-12` and a value to a local variable `c` stored in `fp-12` to `fp-16`.

  ```pascal
  var b, c: integer;
  b := 5;
  c := 6;
  ```

  will be translated into the following `RISC-V` instructions.

  ```assembly
  addi t0, s0, -12
  addi sp, sp, -4
  sw t0, 0(sp)     # push the address to the stack
  li t0, 5
  addi sp, sp, -4
  sw t0, 0(sp)     # push the value to the stack
  lw t0, 0(sp)     # pop the value from the stack
  addi sp, sp, 4
  lw t1, 0(sp)     # pop the address from the stack
  addi sp, sp, 4
  sw t0, 0(t1)     # b = 5
  addi t0, s0, -16
  addi sp, sp, -4
  sw t0, 0(sp)     # push the address to the stack
  li t0, 6
  addi sp, sp, -4
  sw t0, 0(sp)     # push the value to the stack
  lw t0, 0(sp)     # pop the value from the stack
  addi sp, sp, 4
  lw t1, 0(sp)     # pop the address from the stack
  addi sp, sp, 4
  sw t0, 0(t1)     # c = 6
  ```

#### Global Constants

- Declaring a global constant `d`

  ```pascal
  var d: 5;
  ```

  will be translated into the following `RISC-V` instructions.

  ```assembly
  .section    .rodata       # emit rodata section
      .align 2
      .globl d              # emit symbol 'd' to the global symbol table
      .type d, @object
  d:
      .word 5
  ```

#### Local Constants

The same as local variables.

<div align="right">
    <b><a href="#table-of-contents">↥ back to menu</a></b>
</div>

### Expression

- Adding up local variable `b` and local variable `c`, then multiplying with global constant `d`, and finally assigning to global variable `a`

  ```pascal
  a := (b + c) * d;
  ```

  will be translated into the following `RISC-V` instructions.

  ```assembly
  addi sp, sp, -4
  la t0, a          # load the address of 'a'
  sw t0, 0(sp)      # push the address to the stack
  lw t0, -12(s0)    # load the value of 'b'
  addi sp, sp, -4
  sw t0, 0(sp)      # push the value to the stack
  lw t0, -16(s0)    # load the value of 'c'
  addi sp, sp, -4
  sw t0, 0(sp)      # push the value to the stack
  lw t0, 0(sp)      # pop the value from the stack
  addi sp, sp, 4
  lw t1, 0(sp)      # pop the value from the stack
  addi sp, sp, 4
  add t0, t1, t0    # b + c, always save the value in a certain register you choose
  addi sp, sp, -4
  sw t0, 0(sp)      # push the value to the stack
  la t0, d          # load the address of 'd'
  lw t1, 0(t0)      # load the 32-bit value of 'd'
  mv t0, t1
  addi sp, sp, -4
  sw t0, 0(sp)      # push the value to the stack
  lw t0, 0(sp)      # pop the value from the stack
  addi sp, sp, 4
  lw t1, 0(sp)      # pop the value from the stack
  addi sp, sp, 4
  mul t0, t1, t0    # (b + c) * d, always save the value in a certain register you choose
  addi sp, sp, -4
  sw t0, 0(sp)      # push the value to the stack
  lw t0, 0(sp)      # pop the value from the stack
  addi sp, sp, 4
  la t1, 0(sp)      # pop the address from the stack
  addi sp, sp, 4
  sw t0, 0(t1)      # save the value to 'a'
  ```

> The values on the registers may be polluted **after calling a function**, so you should take care of registers if you don't push the values on the registers to the stack every time and there's a function invocation in an expression. The simplest way is saving the registers on the stack in the function prologue and restoring them in the function epilogue.

<div align="right">
    <b><a href="#table-of-contents">↥ back to menu</a></b>
</div>

### Function Declaration and Invocation

- Declaring a function `sum`

  ```pascal
  sum(a,b: integer): integer
  begin
      var c: integer;
      c := a + b;
      return c;
  end
  end
  ```

  will be translated into the following `RISC-V` instructions.

  ```assembly
  .section    .text
      .align 2
      .globl sum
      .type sum, @function
  sum:
      addi sp, sp, -128
      sw ra, 124(sp)
      sw s0, 120(sp)
      addi s0, sp, 128
      sw a0, -12(s0)    # save parameter 'a' in the local stack
      sw a1, -16(s0)    # save parameter 'b' in the local stack
      addi t0, s0, -20  # load the address of 'c'
      addi sp, sp, -4
      sw t0, 0(sp)      # push the address to the stack
      lw t0, -12(s0)    # load the value of 'a'
      addi sp, sp, -4
      sw t0, 0(sp)      # push the value to the stack
      lw t0, -16(s0)    # load the value of 'b'
      addi sp, sp, -4
      sw t0, 0(sp)      # push the value to the stack
      lw t0, 0(sp)      # pop the value from the stack
      addi sp, sp, 4
      lw t1, 0(sp)      # pop the value from the stack
      addi sp, sp, 4
      add t0, t1, t0    # a + b, always save the value in a certain register you choose
      addi sp, sp, -4
      sw t0, 0(sp)      # push the value to the stack
      lw t0, 0(sp)      # pop the value from the stack
      addi sp, sp, 4
      lw t1, 0(sp)      # pop the address from the stack
      addi sp, sp, 4
      sw t0, 0(t1)      # save the value to 'c'
      lw t0, -20(s0)
      addi sp, sp, -4
      sw t0, 0(sp)      # push the value to the stack
      lw t0, 0(sp)      # pop the value from the stack
      addi sp, sp, 4
      mv a0, t0         # load the value of 'c' to the return value register 'a0'
      lw ra, 124(sp)
      lw s0, 120(sp)
      addi sp, sp, 128
      jr ra
      .size sum, .-sum
  ```

- Call function `sum` with local variable `b` and global constant `d`, then assign the result to global variable `a`

  ```pascal
  a := sum(b, d);
  ```

  will be translated into the following `RISC-V` instructions.

  ```assembly
  la t0, a         # load the address of 'a'
  addi sp, sp, -4
  sw t0, 0(sp)     # push the value to the stack
  lw t0, -12(s0)   # load the value of 'b'
  addi sp, sp, -4
  sw t0, 0(sp)     # push the value to the stack
  la t0, d         # load the address of 'd'
  lw t1, 0(t0)     # load the 32-bit value of 'd'
  mv t0, t1
  addi sp, sp, -4
  sw t0, 0(sp)     # push the value to the stack
  lw a1, 0(sp)     # pop the value from the stack to the second argument register 'a1'
  addi sp, sp, 4
  lw a0, 0(sp)     # pop the value from the stack to the first argument register 'a0'
  addi sp, sp, 4
  jal ra, sum      # call function 'sum'
  mv t0, a0        # always move the return value to a certain register you choose
  addi sp, sp, -4
  sw t0, 0(sp)     # push the value to the stack
  lw t0, 0(sp)     # pop the value from the stack
  addi sp, sp, 4
  lw t1, 0(sp)     # pop the address from the stack
  addi sp, sp, 4
  sw t0, 0(t1)     # save the value to 'a'
  ```

> [!note]
> The function argument number in the test case may be larger than **eight**, and there are only `a0`-`a7` registers. You should try to place the remain arguments in other places.

<div align="right">
    <b><a href="#table-of-contents">↥ back to menu</a></b>
</div>

### Simple Statement

It's a little complicated to call an `IO` system call, so we provide you **print** functions and **read** functions in `io.c`. You can compile and link your generated code with the functions by:

```sh
riscv32-unknown-elf-gcc [generated RISC-V assembly file] io.c -o [output ELF file]
```

- Printing a global variable `a`

  ```pascal
  print a;
  ```

  will be translated into the following `RISC-V` instructions.

  ```assembly
  la t0, a
  lw t1, 0(t0)     # load the value of 'a'
  mv t0, t1
  addi sp, sp, -4
  sw t0, 0(sp)     # push the value to the stack
  lw a0, 0(sp)     # pop the value from the stack to the first argument register 'a0'
  addi sp, sp, 4
  jal ra, printInt # call function 'printInt'
  ```

- Read a value and save to a global variable `a`

  ```pascal
  read a;
  ```

  will be translated into the following `RISC-V` instructions.

  ```assembly
  la t0, a         # load the address of 'a'
  addi sp, sp, -4
  sw t0, 0(sp)     # push the address to the stack
  jal ra, readInt  # call function 'readInt'
  lw t0, 0(sp)     # pop the address from the stack
  addi sp, sp, 4
  sw a0, 0(t0)     # save the return value to 'a'
  ```

<div align="right">
    <b><a href="#table-of-contents">↥ back to menu</a></b>
</div>

### Conditional Statement

- Branching according to `a`'s value

  ```pascal
  if ( a <= 40 ) then
  begin
      print a;
  end
  else
  begin
      print b;
  end
  end if
  ```

  will be translated into the following `RISC-V` instructions.

  ```assembly
      la t0, a
      lw t1, 0(t0)          # load the value of 'a'
      mv t0, t1
      addi sp, sp, -4
      sw t0, 0(sp)          # push the value to the stack
      li t0, 40
      addi sp, sp, -4
      sw t0, 0(sp)          # push the value to the stack
      lw t0, 0(sp)          # pop the value from the stack
      addi sp, sp, 4
      lw t1, 0(sp)          # pop the value from the stack
      addi sp, sp, 4
      bgt t1, t0, L2        # if a > 40, jump to L2
  L1:
      la t0, a
      lw t1, 0(t0)          # load the value of 'a'
      mv t0, t1
      addi sp, sp, -4
      sw t0, 0(sp)          # push the value to the stack
      lw a0, 0(sp)          # pop the value from the stack to the first argument register 'a0'
      addi sp, sp, 4
      jal ra, printInt      # call function 'printInt'
      j L3                  # jump to L3
  L2:
      lw t0, -12(s0)        # load the value of 'b'
      addi sp, sp, -4
      sw t0, 0(sp)          # push the value to the stack
      lw a0, 0(sp)          # pop the value from the stack to the first argument register 'a0'
      addi sp, sp, 4
      jal ra, printInt      # call function 'printInt'
  L3:
  ```

<div align="right">
    <b><a href="#table-of-contents">↥ back to menu</a></b>
</div>

### For Statement and While Statement

- Looping until b >= 8

  ```pascal
  while b < 8 do
  begin
      print b;
      b := b + 1;
  end
  end do
  ```

  will be translated into the following `RISC-V` instructions.

  ```assembly
  L3:
      lw t0, -12(s0)        # load the value of 'b'
      addi sp, sp, -4
      sw t0, 0(sp)          # push the value to the stack
      li t0, 8
      addi sp, sp, -4
      sw t0, 0(sp)          # push the value to the stack
      lw t0, 0(sp)          # pop the value from the stack
      addi sp, sp, 4
      lw t1, 0(sp)          # pop the value from the stack
      addi sp, sp, 4
      bge t1, t0, L5        # if b >= 8, exit the loop
  L4:
      lw t0, -12(s0)        # load the value of 'b'
      addi sp, sp, -4
      sw t0, 0(sp)          # push the value to the stack
      lw a0, 0(sp)          # pop the value from the stack to the first argument register 'a0'
      addi sp, sp, 4
      jal ra, printInt      # call function 'printInt'
      addi t0, s0, -12      # load the address of 'b'
      addi sp, sp, -4
      sw t0, 0(sp)          # push the address to the stack
      lw t0, -12(s0)        # load the value of 'b'
      addi sp, sp, -4
      sw t0, 0(sp)          # push the value to the stack
      li t0, 1
      addi sp, sp, -4
      sw t0, 0(sp)          # push the value to the stack
      lw t0, 0(sp)          # pop the value from the stack
      addi sp, sp, 4
      lw t1, 0(sp)          # pop the value from the stack
      addi sp, sp, 4
      add t0, t1, t0        # b + 1, always save the value in a certain register you choose
      addi sp, sp, -4
      sw t0, 0(sp)          # push the value to the stack
      lw t0, 0(sp)          # pop the value from the stack
      addi sp, sp, 4
      lw t1, 0(sp)          # pop the address from the stack
      addi sp, sp, 4
      sw t0, 0(t1)          # save the value to 'b'
      j L3                  # jump back to loop condition
  L5:
  ```

- Looping with loop variable `i`

  ```pascal
  for i := 10 to 13 do
  begin
      print i;
  end
  end do
  ```

  will be translated into the following `RISC-V` instructions.

  ```assembly
      addi t0, s0, -20
      addi sp, sp, -4
      sw t0, 0(sp)          # push the address of the loop variable to the stack
      li t0, 10
      addi sp, sp, -4
      sw t0, 0(sp)          # push the value to the stack
      lw t0, 0(sp)          # pop the value from the stack
      addi sp, sp, 4
      lw t1, 0(sp)          # pop the address from the stack
      addi sp, sp, 4
      sw t0, 0(t1)          # save the loop variable in the local stack
  L6:
      lw t0, -20(s0)        # load the value of 'i'
      addi sp, sp, -4
      sw t0, 0(sp)          # push the value to the stack
      li t0, 13
      addi sp, sp, -4
      sw t0, 0(sp)          # push the value to the stack
      lw t0, 0(sp)          # pop the value from the stack
      addi sp, sp, 4
      lw t1, 0(sp)          # pop the value from the stack
      addi sp, sp, 4
      bge t1, t0, L8        # if i >= 13, exit the loop
  L7:
      lw t0, -20(s0)        # load the value of 'i'
      addi sp, sp, -4
      sw t0, 0(sp)          # push the value to the stack
      lw a0, 0(sp)          # pop the value from the stack to the first argument register 'a0'
      addi sp, sp, 4
      jal ra, printInt      # call function 'printInt'
      addi t0, s0, -20      # load the address of 'i'
      addi sp, sp, -4
      sw t0, 0(sp)          # push the address to the stack
      lw t0, -20(s0)        # load the value of 'i'
      addi sp, sp, -4
      sw t0, 0(sp)          # push the value to the stack
      li t0, 1
      addi sp, sp, -4
      sw t0, 0(sp)          # push the value to the stack
      lw t0, 0(sp)          # pop the value from the stack
      addi sp, sp, 4
      lw t1, 0(sp)          # pop the value from the stack
      addi sp, sp, 4
      add t0, t1, t0        # i + 1, always save the value in a certain register you choose
      addi sp, sp, -4
      sw t0, 0(sp)          # push the value to the stack
      lw t0, 0(sp)          # pop the value from the stack
      addi sp, sp, 4
      lw t1, 0(sp)          # pop the address from the stack
      addi sp, sp, 4
      sw t0, 0(t1)          # save the value to 'i'
      j L6                  # jump back to loop condition
  L8:
  ```

<div align="right">
    <b><a href="#table-of-contents">↥ back to menu</a></b>
</div>

### Combining All Examples Above

<details>
<summary>Click to expand!</summary>

```pascal
// test1.p
test1;

var a: integer;
var d: 5;

sum(a,b: integer): integer
begin
    var c: integer;
    c := a + b;
    return c;
end
end

begin

var b, c: integer;
b := 5;
c := 6;

read a;
print a;

a := sum(b, d);
print a;

a := (b + c) * d;
print a;

if ( a <= 40 ) then
begin
    print a;
end
else
begin
    print b;
end
end if

while b < 8 do
begin
    print b;
    b := b + 1;
end
end do

for i := 10 to 13 do
begin
    print i;
end
end do

end
end
```

will be translated into the following `RISC-V` instructions.

```assembly
    .file "test1.p"
    .option nopic
.comm a, 4, 4             # emit object 'a' to .bss section with size = 4, align = 4
.section    .rodata       # emit rodata section
    .align 2
    .globl d              # emit symbol 'd' to the global symbol table
    .type d, @object
d:
    .word 5
.section    .text
    .align 2
    .globl sum
    .type sum, @function
sum:
    addi sp, sp, -128
    sw ra, 124(sp)
    sw s0, 120(sp)
    addi s0, sp, 128
    sw a0, -12(s0)        # save parameter 'a' in the local stack
    sw a1, -16(s0)        # save parameter 'b' in the local stack
    addi t0, s0, -20      # load the address of 'c'
    addi sp, sp, -4
    sw t0, 0(sp)          # push the address to the stack
    lw t0, -12(s0)        # load the value of 'a'
    addi sp, sp, -4
    sw t0, 0(sp)          # push the value to the stack
    lw t0, -16(s0)        # load the value of 'b'
    addi sp, sp, -4
    sw t0, 0(sp)          # push the value to the stack
    lw t0, 0(sp)          # pop the value from the stack
    addi sp, sp, 4
    lw t1, 0(sp)          # pop the value from the stack
    addi sp, sp, 4
    add t0, t1, t0        # a + b, always save the value in a certain register you choose
    addi sp, sp, -4
    sw t0, 0(sp)          # push the value to the stack
    lw t0, 0(sp)          # pop the value from the stack
    addi sp, sp, 4
    lw t1, 0(sp)          # pop the address from the stack
    addi sp, sp, 4
    sw t0, 0(t1)          # save the value to 'c'
    lw t0, -20(s0)
    addi sp, sp, -4
    sw t0, 0(sp)          # push the value to the stack
    lw t0, 0(sp)          # pop the value from the stack
    addi sp, sp, 4
    mv a0, t0             # load the value of 'c' to the return value register 'a0'
    lw ra, 124(sp)
    lw s0, 120(sp)
    addi sp, sp, 128
    jr ra
    .size sum, .-sum
.section    .text
    .align 2
    .globl main           # emit symbol 'main' to the global symbol table
    .type main, @function
main:
    # in the function prologue
    addi sp, sp, -128     # move stack pointer to lower address to allocate a new stack
    sw ra, 124(sp)        # save return address of the caller function in the current stack
    sw s0, 120(sp)        # save frame pointer of the last stack in the current stack
    addi s0, sp, 128      # move frame pointer to the bottom of the current stack
    # b = 5
    addi t0, s0, -12
    addi sp, sp, -4
    sw t0, 0(sp)          # push the address to the stack
    li t0, 5
    addi sp, sp, -4
    sw t0, 0(sp)          # push the value to the stack
    lw t0, 0(sp)          # pop the value from the stack
    addi sp, sp, 4
    lw t1, 0(sp)          # pop the address from the stack
    addi sp, sp, 4
    sw t0, 0(t1)          # b = 5
    # c = 6
    addi t0, s0, -16
    addi sp, sp, -4
    sw t0, 0(sp)          # push the address to the stack
    li t0, 6
    addi sp, sp, -4
    sw t0, 0(sp)          # push the value to the stack
    lw t0, 0(sp)          # pop the value from the stack
    addi sp, sp, 4
    lw t1, 0(sp)          # pop the address from the stack
    addi sp, sp, 4
    sw t0, 0(t1)          # c = 6
    # read a
    la t0, a              # load the address of 'a'
    addi sp, sp, -4
    sw t0, 0(sp)          # push the address to the stack
    jal ra, readInt       # call function 'readInt'
    lw t0, 0(sp)          # pop the address from the stack
    addi sp, sp, 4
    sw a0, 0(t0)          # save the return value to 'a'
    # print a
    la t0, a
    lw t1, 0(t0)          # load the value of 'a'
    mv t0, t1
    addi sp, sp, -4
    sw t0, 0(sp)          # push the value to the stack
    lw a0, 0(sp)          # pop the value from the stack to the first argument register 'a0'
    addi sp, sp, 4
    jal ra, printInt      # call function 'printInt'
    # a = sum(b, d)
    la t0, a              # load the address of 'a'
    addi sp, sp, -4
    sw t0, 0(sp)          # push the value to the stack
    lw t0, -12(s0)        # load the value of 'b'
    addi sp, sp, -4
    sw t0, 0(sp)          # push the value to the stack
    la t0, d              # load the address of 'd'
    lw t1, 0(t0)          # load the 32-bit value of 'd'
    mv t0, t1
    addi sp, sp, -4
    sw t0, 0(sp)          # push the value to the stack
    lw a1, 0(sp)          # pop the value from the stack to the second argument register 'a1'
    addi sp, sp, 4
    lw a0, 0(sp)          # pop the value from the stack to the first argument register 'a0'
    addi sp, sp, 4
    jal ra, sum           # call function 'sum'
    mv t0, a0             # always move the return value to a certain register you choose
    addi sp, sp, -4
    sw t0, 0(sp)          # push the value to the stack
    lw t0, 0(sp)          # pop the value from the stack
    addi sp, sp, 4
    lw t1, 0(sp)          # pop the address from the stack
    addi sp, sp, 4
    sw t0, 0(t1)          # save the value to 'a'
    la t0, a
    lw a0, 0(t0)
    jal ra, printInt
    # a = (b + c) * d
    addi sp, sp, -4
    la t0, a              # load the address of 'a'
    sw t0, 0(sp)          # push the address to the stack
    lw t0, -12(s0)        # load the value of 'b'
    addi sp, sp, -4
    sw t0, 0(sp)          # push the value to the stack
    lw t0, -16(s0)        # load the value of 'c'
    addi sp, sp, -4
    sw t0, 0(sp)          # push the value to the stack
    lw t0, 0(sp)          # pop the value from the stack
    addi sp, sp, 4
    lw t1, 0(sp)          # pop the value from the stack
    addi sp, sp, 4
    add t0, t1, t0        # b + c, always save the value in a certain register you choose
    addi sp, sp, -4
    sw t0, 0(sp)          # push the value to the stack
    la t0, d              # load the address of 'd'
    lw t1, 0(t0)          # load the 32-bit value of 'd'
    mv t0, t1
    addi sp, sp, -4
    sw t0, 0(sp)          # push the value to the stack
    lw t0, 0(sp)          # pop the value from the stack
    addi sp, sp, 4
    lw t1, 0(sp)          # pop the value from the stack
    addi sp, sp, 4
    mul t0, t1, t0        # (b + c) * d, always save the value in a certain register you choose
    addi sp, sp, -4
    sw t0, 0(sp)          # push the value to the stack
    lw t0, 0(sp)          # pop the value from the stack
    addi sp, sp, 4
    lw t1, 0(sp)          # pop the address from the stack
    addi sp, sp, 4
    sw t0, 0(t1)          # save the value to 'a'
    la t0, a
    lw a0, 0(t0)
    jal ra, printInt
    # condition example
    la t0, a
    lw t1, 0(t0)          # load the value of 'a'
    mv t0, t1
    addi sp, sp, -4
    sw t0, 0(sp)          # push the value to the stack
    li t0, 40
    addi sp, sp, -4
    sw t0, 0(sp)          # push the value to the stack
    lw t0, 0(sp)          # pop the value from the stack
    addi sp, sp, 4
    lw t1, 0(sp)          # pop the value from the stack
    addi sp, sp, 4
    bgt t1, t0, L2        # if a > 40, jump to L2
L1:
    la t0, a
    lw t1, 0(t0)          # load the value of 'a'
    mv t0, t1
    addi sp, sp, -4
    sw t0, 0(sp)          # push the value to the stack
    lw a0, 0(sp)          # pop the value from the stack to the first argument register 'a0'
    addi sp, sp, 4
    jal ra, printInt      # call function 'printInt'
    j L3                  # jump to L3
L2:
    lw t0, -12(s0)        # load the value of 'b'
    addi sp, sp, -4
    sw t0, 0(sp)          # push the value to the stack
    lw a0, 0(sp)          # pop the value from the stack to the first argument register 'a0'
    addi sp, sp, 4
    jal ra, printInt      # call function 'printInt'
L3:
    # while loop example
    lw t0, -12(s0)        # load the value of 'b'
    addi sp, sp, -4
    sw t0, 0(sp)          # push the value to the stack
    li t0, 8
    addi sp, sp, -4
    sw t0, 0(sp)          # push the value to the stack
    lw t0, 0(sp)          # pop the value from the stack
    addi sp, sp, 4
    lw t1, 0(sp)          # pop the value from the stack
    addi sp, sp, 4
    bge t1, t0, L5        # if b >= 8, exit the loop
L4:
    lw t0, -12(s0)        # load the value of 'b'
    addi sp, sp, -4
    sw t0, 0(sp)          # push the value to the stack
    lw a0, 0(sp)          # pop the value from the stack to the first argument register 'a0'
    addi sp, sp, 4
    jal ra, printInt      # call function 'printInt'
    addi t0, s0, -12      # load the address of 'b'
    addi sp, sp, -4
    sw t0, 0(sp)          # push the address to the stack
    lw t0, -12(s0)        # load the value of 'b'
    addi sp, sp, -4
    sw t0, 0(sp)          # push the value to the stack
    li t0, 1
    addi sp, sp, -4
    sw t0, 0(sp)          # push the value to the stack
    lw t0, 0(sp)          # pop the value from the stack
    addi sp, sp, 4
    lw t1, 0(sp)          # pop the value from the stack
    addi sp, sp, 4
    add t0, t1, t0        # b + 1, always save the value in a certain register you choose
    addi sp, sp, -4
    sw t0, 0(sp)          # push the value to the stack
    lw t0, 0(sp)          # pop the value from the stack
    addi sp, sp, 4
    lw t1, 0(sp)          # pop the address from the stack
    addi sp, sp, 4
    sw t0, 0(t1)          # save the value to 'b'
    j L3                  # jump back to loop condition
L5:
    # for loop example
    addi t0, s0, -20
    addi sp, sp, -4
    sw t0, 0(sp)          # push the address of the loop variable to the stack
    li t0, 10
    addi sp, sp, -4
    sw t0, 0(sp)          # push the value to the stack
    lw t0, 0(sp)          # pop the value from the stack
    addi sp, sp, 4
    lw t1, 0(sp)          # pop the address from the stack
    addi sp, sp, 4
    sw t0, 0(t1)          # save the loop variable in the local stack
L6:
    lw t0, -20(s0)        # load the value of 'i'
    addi sp, sp, -4
    sw t0, 0(sp)          # push the value to the stack
    li t0, 13
    addi sp, sp, -4
    sw t0, 0(sp)          # push the value to the stack
    lw t0, 0(sp)          # pop the value from the stack
    addi sp, sp, 4
    lw t1, 0(sp)          # pop the value from the stack
    addi sp, sp, 4
    bge t1, t0, L8        # if i >= 13, exit the loop
L7:
    lw t0, -20(s0)        # load the value of 'i'
    addi sp, sp, -4
    sw t0, 0(sp)          # push the value to the stack
    lw a0, 0(sp)          # pop the value from the stack to the first argument register 'a0'
    addi sp, sp, 4
    jal ra, printInt      # call function 'printInt'
    addi t0, s0, -20      # load the address of 'i'
    addi sp, sp, -4
    sw t0, 0(sp)          # push the address to the stack
    lw t0, -20(s0)        # load the value of 'i'
    addi sp, sp, -4
    sw t0, 0(sp)          # push the value to the stack
    li t0, 1
    addi sp, sp, -4
    sw t0, 0(sp)          # push the value to the stack
    lw t0, 0(sp)          # pop the value from the stack
    addi sp, sp, 4
    lw t1, 0(sp)          # pop the value from the stack
    addi sp, sp, 4
    add t0, t1, t0        # i + 1, always save the value in a certain register you choose
    addi sp, sp, -4
    sw t0, 0(sp)          # push the value to the stack
    lw t0, 0(sp)          # pop the value from the stack
    addi sp, sp, 4
    lw t1, 0(sp)          # pop the address from the stack
    addi sp, sp, 4
    sw t0, 0(t1)          # save the value to 'i'
    j L6                  # jump back to loop condition
L8:
    # in the function epilogue
    lw ra, 124(sp)        # load return address saved in the current stack
    lw s0, 120(sp)        # move frame pointer back to the bottom of the last stack
    addi sp, sp, 128      # move stack pointer back to the top of the last stack
    jr ra                 # jump back to the caller function
    .size main, .-main
```

</details>

### Bonus

You need to generate `RISC-V` instructions for `boolean type`, `array type`, `string type`, and `real type` for the extra points. The code generation for additional types will be tested **with** declarations, statements and expressions. The following are some hints for the bonus.

#### Boolean Type

Just treat the `true` and `false` as `1` and `0`.

#### Array Type

For simplicity, you can pass the array variables by value.

If you prefer to pass them by address like what C does, remember that you'll need to access them with two memory loads. Additionally, note that a local array has a different meaning from a parameter array: for the former, the entire array is held on the stack, while for the latter, only the address of the first element is stored.

#### String Type

There is no string concatenation in test cases, so you don't need to allocate dynamic memory for a string variable. A unique string literal will be allocated only once throughout the program, and different variables refer to it through its address (or symbol).

- Defining a local string variable with symbol 'st' (the symbol does not have to be the same as the variable name) will be translated into the following `RISC-V` instructions.

  ```assembly
      .section	.rodata
      .align	2
  st:
      .string	"hello"
  ```

- Referencing a local string variable with symbol 'st' will be translated into the following `RISC-V` instructions.

  ```assembly
  la a0, st
  ```

> [!note]
> Alternatively, you can use the following instructions to load 'st':
>
> ```assembly
>   lui t0, %hi(st)
>   addi a0, t0, %lo(st)
> ```
>
> However, the pseudo-instruction `la` is more readable, and it handles _pic_/_nopic_ for you [^1].

[^1]: [RISC-V Assembly Programmer's Manual - Load Address](https://github.com/riscv-non-isa/riscv-asm-manual/blob/master/riscv-asm.md#load-address)

#### Real Type

You should use floating-point registers and floating-point instructions for real type code generation. In RV32, float-point numbers are single precision. Check them out in [RISC-V ISA Specification, p.93](https://drive.google.com/file/d/1s0lZxUZaa7eV_O0_WsZzaurFLLww7ou5/view?pli=1).

- Defining a local real type variable 'rv' will be translated into the following `RISC-V` instructions.

  ```assembly
      .section	.rodata
      .align	2
  rv:
      .float	1.1
          .
          .
          .
  main:
      lui t0, %hi(rv)
      flw ft0, %lo(rv)(t0)
      fsw ft0, -24(s0)
  ```

Floating-point constants cannot be loaded directly into a register as an immediate; they are treated similarly as the strings by first allocated in the `rodata` section.

- Adding two real type variables will be translated into the following `RISC-V` instructions.

  ```assembly
  flw	ft0, -24(s0)
  flw	ft1, -24(s0)
  fadd.s  ft0, ft1, ft0
  fsw	ft0, -24(s0)
  ```

<div align="right">
    <b><a href="#table-of-contents">↥ back to menu</a></b>
</div>

## Implementation Hints

### Using `C` compiler that targets RISC-V

If you have no idea what instructions will be generated from a `P` program, you may write a corresponding `C` program and run the following command to generate `RISC-V` instructions using a `C` compiler. Check out `[output assembly file]` to see what instructions were generated.

```sh
riscv32-unknown-elf-gcc -c -S [input C file] -o [output assembly file]
```

<div align="right">
    <b><a href="#table-of-contents">↥ back to menu</a></b>
</div>

### Debugging by separating the compilation phases

If you encounter a `Couldn't open ELF program error` in failed tests, it indicates a failure to generate the executable successfully. This could be due to an error in the compiler itself or incorrect assembly code generation. To debug this issue, you can separate the compilation phases by executing `src/compiler`, `riscv32-unknown-elf-gcc`, and `spike` individually.

<div align="right">
    <b><a href="#table-of-contents">↥ back to menu</a></b>
</div>

## Project Structure

- `README.md`
- /src
  - Makefile
  - `scanner.l`
  - `parser.y`
  - /include
    - /AST
    - /semantic
    - /visitor
    - /codegen
      - CodeGenerator.hpp - for code generation in visitor pattern version
  - /lib
    - /AST
    - /semantic
    - /visitor
    - /codegen
      - CodeGenerator.cpp - for code generation in visitor pattern version
  - Other modules you may add
- /report
  - `README.md`

In this assignment, you have to do the following tasks:

- Revise `CodeGenerator.hpp` and `CodeGenerator.cpp`, and add some modules to generate the `RISC-V` instructions.
- Write the report in `report/README.md`. The report should describe your feedback about `hw5` For example, is the spec and the tutorial clear? Is `hw5` too easy?

If you want to preview your report in GitHub style markdown before pushing to GitHub, [`grip`](https://github.com/joeyespo/grip) might be the tool you need.

<div align="right">
    <b><a href="#table-of-contents">↥ back to menu</a></b>
</div>

## Assessment Rubrics (Grading)

Total of 119 points.

- Passing all test cases (95 pts)
  - hidden test cases (47.5 pts)
- Report (5 pts)
  - 0: empty
  - 1: bad
  - 3: normal
  - 4: good
  - 5: excellent
- Bonus (Total of 19 pts)
  - Code generation for boolean types (4 pts)
    - hidden test case (2 pts)
  - Code generation for array types (6 pts)
    - hidden test case (3 pts)
  - Code generation for string types (3 pts)
    - hidden test case (1.5 pts)
  - Code generation for real types (6 pts)
    - hidden test case (3 pts)

<div align="right">
    <b><a href="#table-of-contents">↥ back to menu</a></b>
</div>

## Build and Execute

> [!important]
> If you're on macOS, please use the emulator exclusively for testing your homework.

- Get Hw5 docker image: `make docker-pull`
- Activate docker environment: `./activate_docker.sh`
- Build: `make`
- Execute: `./compiler [input file] --save-path [save path]`
- Test: `make test`
- Test on board: `make board`

### Build Project

TA would use `src/Makefile` to build your project by simply typing `make clean && make`. You have to make sure that it will generate an executable named '`compiler`'. **No further grading will be made if the `make` process fails or the executable '`compiler`' is not found.**

### Test your compiler with the simulator

We provide all the test cases in the `test` folder. Simply type `make test` to test your compiler. The grade you got will be shown on the terminal. You can also check `diff.txt` in `test/result` folder to know the diff result between the outputs of your compiler and the sample solutions.

### Simulator Commands

The `RISC-V` simulator has been installed in the docker image. You may install it on your environment. The following commands show how to generate the executable and run the executable on the `RISC-V` simulator.

- Compile the generated `RISC-V` instructions to the `Executable and Linkable Format (ELF)` file.

```sh
riscv32-unknown-elf-gcc -o [output ELF file] [input RISC-V instruction file]
```

- Run the `ELF` file on the simulator `spike`.

```sh
spike --isa=rv32gc /risc-v/riscv32-unknown-elf/bin/pk [ELF file]
```

### Test your compiler with the RISC-V development board

> [!note]
> The following test does not count for any points, and you can choose to implement it or not.

We provide the [`Seeed Studio Sipeed Longan Nano`](https://www.seeedstudio.com/Sipeed-Longan-Nano-RISC-V-GD32VF103CBT6-Development-Board-p-4205.html) development board for testing your compiler. Your compiler will be tested with the following tests:

1. Global variable
2. Local variable
3. Global constant
4. Expression
5. Function definition and function invocation
6. Function invocation in an expression
7. Conditional statement
8. For statement
9. While statement

The `P` program which is going to be compiled by your compiler will call the functions written in `C` to test the correctness of your compiler, so all the above tests are based on the function declarations and the function invocations.

We use `USB DFU` download to transfer the compiled executable to the development board. First, connect your board to your computer using a USB TYPE-C cable. To enter DFU mode:

1. Hold down the `BOOT` button.
2. Press the `RESET` button to restart the development board.
3. Release the `BOOT` button.

If you're using WSL, you need to connect the board to WSL:

1. In _powershell.exe_ or _cmd.exe,_ using `usbipd list` to find the USBID for the board.
2. Share the board with WSL using `usbipd bind -b 1-1` and `usbipd attach --busid 1-1 --wsl`.

```sh
> usbipd list
Connected:
BUSID  VID:PID    DEVICE                                                        STATE
1-1    28e9:0189  Unknown device                                                Not shared

> usbipd bind -b 1-1
> usbipd list
Connected:
BUSID  VID:PID    DEVICE                                                        STATE
1-1    28e9:0189  Unknown device                                                Shared

> usbipd attach --busid 1-1 --wsl
usbipd: info: Using WSL distribution 'Ubuntu' to attach; the device will be available in all WSL 2 distributions.
usbipd: info: Using IP address 172.20.224.1 to reach the host.
> usbipd list
Connected:
BUSID  VID:PID    DEVICE                                                        STATE
1-1    28e9:0189  Unknown device                                                Attached
```

For more details on this issue, please refer to the [Connect USB devices](https://learn.microsoft.com/en-us/windows/wsl/connect-usb) article.

Then, verify that the board is connected to Linux via `lsusb`, installed using `sudo apt-get install usbutils`.

```sh
$ lsusb
Bus 001 Device 009: ID 28e9:0189 GDMicroelectronics GD32 DFU Bootloader (Longan Nano)
```

After connecting the board to Linux, enter the container with the additional argument `./activate_docker.sh --board`, then check USB permission using `dfu-util -l`.

```sh
student@compiler-course:~/hw$ dfu-util -l
...

Found DFU: [28e9:0189] ver=1000, devnum=11, cfg=1, intf=0, path="1-1", alt=1, name="@Option Bytes  /0x1FFFF800/01*016 g", serial="??"
Found DFU: [28e9:0189] ver=1000, devnum=11, cfg=1, intf=0, path="1-1", alt=0, name="@Internal Flash  /0x08000000/512*002Kg", serial="??"
```

If you encounter the error `dfu-util: Cannot open DFU device 28e9:0189`, you should add some rules for `udev`. Here's an example for Ubuntu:

1. Create a file named `/etc/udev/rules.d/70-ttyusb.rules`.

   ```conf
   SUBSYSTEM=="usb", ATTRS{idVendor}=="28e9", ATTRS{idProduct}=="0189", MODE="0666"
   SUBSYSTEM=="usb_device", ATTRS{idVendor}=="28e9", ATTRS{idProduct}=="0189", MODE="0666"
   ```

2. Restart `udev` using `sudo service udev restart`.
3. Replug the board.

For more details on this issue, please refer to the ["Cannot open DFU device 28e9:0189" · Issue #2 · riscv-mcu/gd32-dfu-utils](https://github.com/riscv-mcu/gd32-dfu-utils/issues/2#issuecomment-838090469) article.

Afterwards, simply type `make board` to test your compiler with the board. The results will be displayed on the LCD of the board.

<div align="right">
    <b><a href="#table-of-contents">↥ back to menu</a></b>
</div>

## Submitting the Assignment

You should push all your commits to the designated repository (hw5-\<Name of your GitHub account\>) under the GitHub organization by the deadline (given in the very beginning of this assignment description).
At any point, you may save your work and push commits to your repository. You **must** commit your final version to the **main branch**, and we will grade the commit which is last pushed on your main branch. The **push time** of that commit will be your submission time. As late submissions are not allowed for this assignment, any commits made after the deadline will not be graded.

In addition, homework assignments **must be individual work**. If we detect what we consider to be intentional plagiarism in any assignment, the assignment will receive reduced or, usually, **zero credit**.

<div align="right">
    <b><a href="#table-of-contents">↥ back to menu</a></b>
</div>
