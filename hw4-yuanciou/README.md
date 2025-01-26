# Project Assignment 4 - Semantic Analyses

**Introduction to Compiler Design by Prof. Yi-Ping You**

### Due Date: 23:59, December 13, 2024

---

**Table of Contents**

- [Project Assignment 4 - Semantic Analyses](#project-assignment-4---semantic-analyses)
  - [Overview](#overview)
  - [Assignment Description](#assignment-description)
    - [0x00. Introduction to Symbol Table](#0x00-introduction-to-symbol-table)
      - [Implementation](#implementation)
      - [Operations](#operations)
      - [Scope Management](#scope-management)
      - [Scope Rules](#scope-rules)
      - [Output Format](#output-format)
      - [Pseudocomments](#pseudocomments)
    - [0x01. Semantic Analysis](#0x01-semantic-analysis)
    - [0x02. Semantic Definition](#0x02-semantic-definition)
      - [Symbol Table](#symbol-table)
      - [Variable Declaration](#variable-declaration)
      - [Variable Reference](#variable-reference)
      - [Binary/Unary operator](#binaryunary-operator)
      - [Type Coercion and Comparison](#type-coercion-and-comparison)
      - [Function Invocation](#function-invocation)
      - [Print and Read Statement](#print-and-read-statement)
      - [Assignment](#assignment)
      - [If and While](#if-and-while)
      - [For](#for)
      - [Return](#return)
  - [Implementation Hints](#implementation-hints)
    - [Recommended Workflow of Developing](#recommended-workflow-of-developing)
    - [Symbol Table Construction](#symbol-table-construction)
    - [Type Information Propagation](#type-information-propagation)
    - [Error Report](#error-report)
  - [What Should Your Parser Do?](#what-should-your-parser-do)
  - [Project Structure](#project-structure)
  - [Assessment Rubrics (Grading)](#assessment-rubrics-grading)
  - [Build and Execute](#build-and-execute)
    - [Build Project](#build-project)
    - [Test your parser](#test-your-parser)
  - [Submitting the Assignment](#submitting-the-assignment)

## Overview

In this assignment, you will extend your parser to perform semantic analyses for a given program written in `P` language using the information recorded in the AST, which has been constructed in the previous assignment.

This assignment requires you to construct a symbol table for performing semantic analyses this time and code generation in the last assignment.
You should design it using feasible data structures.

> [!important]
> DO NOT use string for everything, unless you're obsessed with parsing strings for extracting information from them.

## Assignment Description

The purpose of this assignment is to get you acquainted with semantic analysis. You'll learn what should be done to achieve it and how does it work.

### 0x00. Introduction to Symbol Table

In the previous assignment, we have constructed an AST for the given program. In a program, the most common behavior is that we declare some variables and use them later.
However, it's quite painful to walk back and forth between a declaration node and a variable reference node in the AST.

As a result, we need to maintain a data structure, symbol table, which stores some information when we encounter a declaration. After that, we can directly get information from the symbol table when a reference is analyzed instead of going back to find the declaration.

A symbol table is used for the following purposes:

- To pass information from declarations to uses.
- To verify if a symbol has been declared before uses.
- To help type checking when analyzing assignments, operations, and return statement.

**Components**

A symbol table is simply a table that contains entries for each name of program, functions, or variables. Each entry consists of the following components:

| Field     | Description                                                                                                                                                                                        |
| --------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Name      | The name of the symbol. Each symbol have the length between 1 to 32. The extra part of an identifier will be discarded.                                                                            |
| Kind      | The name type of the symbol. There are **six** kinds of symbols: program, function, parameter, variable, loop_var, and constant.                                                                   |
| Level     | The scope level of the symbol. 0 represents the global scope. Local scope levels start from 1, and the scope level is incremented at the start of a scope and decremented at the end of the scope. |
| Type      | The type of the symbol. Each symbol is of types integer, real, boolean, string, or the signature of an array. (Note that this field can be used for the return type of a function.)                |
| Attribute | Other attributes of the symbol, the value of a constant, or list of the types of the formal parameters of a function.                                                                              |

#### Implementation

A symbol table can be implemented in one of the following ways to represent entries in it:

- Linear list
- Binary search tree
- Hash table

#### Operations

A symbol table should provide the following operations:

- insert
  - used to add a new symbol declaration in the symbol table
- lookup
  - used to search a identifier in the symbol table for performing semantic analyses

#### Scope Management

In general, we won't just have one giant symbol table for each program. There is a concept of scope in the `P` language. Scope refers to the visibility of symbols. That is, different parts of the program located in different scopes may not see or use symbols in other scopes.

A scope corresponds to a symbol table. `ProgramNode`, `FunctionNode`, `ForNode` and `CompoundStatementNode` are nodes that form a scope. Therefore, your parser should create a new symbol table when encountering one of these nodes and destroy it when leaving the node.

Following are the special rules that you should be careful of:

- A `FunctionNode` should share the same symbol table with its body (`CompoundStatementNode`). More specifically, parameters of a `FunctionNode` should be declared in the same symbol table with those declared in the `CompoundStatementNode`.
- A `ForNode` contains a symbol table for the loop variable. That is, there is a scope (symbol table) in a `ForNode` with **only one symbol of a loop variable**.

#### Scope Rules

After entering a node that forms a scope, we may encounter some declarations. Then we have to extract information from the declaration and store it in the current symbol table.

When constructing an entry in a symbol table, there are some rules to conform:

- Scope rules are similar to C.
- Name must be unique within a given scope. The identifier designates the entity declared closest to it; that is, the identifier declared in the outer scope is hidden by the one declared in the inner scope.
  - Unlike the normal variable, the symbol of a loop variable **CANNOT** be redeclared, whether it's in the same scope or the inner scope.
  - If there are multiple declarations with the same identifier in the same scope, only the first declaration will be placed in the symbol table.
- Declarations within a compound statement, a function, or a for statement are local to the statements in the same block or the inner block, and no longer exist after exiting the block in which it's declared.

If there is a violation, your parser should report a semantic error. (we'll discuss it later.)

#### Output Format

> [!note]
>
> - Your parser should dump the symbol table to **`stdout`**.
> - To simplify testing, symbols in the same symbol table should be dumped in the order of their appearance, i.e., the order in which they are added to the symbol table.

Format of each component:

- Name
  - just characters
- Kind
  - 6 kinds of symbols with respective strings: `program`, `function`, `parameter`, `variable`, `loop_var`, and `constant`
- Level
  - 0: `"0(global)"`
  - other level: `n(local)`
- Type
  - just type string, same rule as hw3
- Attribute
  - constant
    - just string of constant
  - types of formal parameters
    - type strings separated by `", "` (e.g., `real, real [2]`)

Format of whole table:

```c
void dumpDemarcation(const char chr) {
  for (size_t i = 0; i < 110; ++i) {
    printf("%c", chr);
  }
  puts("");
}

void dumpSymbol(void) {
  dumpDemarcation('=');
  printf("%-33s%-11s%-11s%-17s%-11s\n", "Name", "Kind", "Level", "Type",
                                        "Attribute");
  dumpDemarcation('-')
  {
    printf("%-33s", "func");
    printf("%-11s", "function");
    printf("%d%-10s", 0, "(global)");
    printf("%-17s", "boolean");
    printf("%-11s", "integer, real [2][3]");
    puts("");
  }
  dumpDemarcation('-')
}
```

**Example:**

Assume we have the following function in a `P` program:

```pascal
foo()
begin
    // constant
    var int1, int2 : 10;
    var str : "Gimme Gimme Gimme!!";
    var bool : true;
    var float : 2.56;
    var scientific : 111.111E-3;
    var octal : 0777;
end
end
```

Then the symbol table of the scope formed by the `CompoundStatementNode` in this function `foo()`:

```text
==============================================================================================================
Name                             Kind       Level      Type             Attribute
--------------------------------------------------------------------------------------------------------------
int1                             constant   1(local)   integer          10
int2                             constant   1(local)   integer          10
str                              constant   1(local)   string           Gimme Gimme Gimme!!
bool                             constant   1(local)   boolean          true
float                            constant   1(local)   real             2.560000
scientific                       constant   1(local)   real             0.111111
octal                            constant   1(local)   integer          511
--------------------------------------------------------------------------------------------------------------
```

#### Pseudocomments

In the first assignment, we have defined:

- `S`
  - `&S+` turns on source program listing, and `&S-` turns it off.
- `T`
  - `&T+` turns on token (which will be returned to the parser) listing, and `&T-` turns it off.

In this assignment, one more option is added:

- `D`
  - Dump the contents of the symbol table associated with a block when exiting from that block.
  - `&D+` turns on symbol table dumping, and `&D-` turns it off.
  - By default, this option is on.
  - In test cases, this option won't be turned on/off in the middle of the program, it will only be set before the `ProgramName`.

### 0x01. Semantic Analysis

Now we have a basic grasp of what the symbol table is. Let's see how to use it in semantic analysis.

Before that, we need to talk about when to perform semantic analyses. Basically, semantic analyses can be separated into two parts by the performed order. One is performed in _pre-order_, while another one is in _post-order_. There is only one semantic analysis that should be performed in pre-order, that is symbol redeclaration. Except for symbol redeclaration, all other semantic analyses are performed in post-order.

In this assignment, you have to implement a semantic analyzer to perform semantic analyses on a given program by traversing the AST of it. When meeting a node, your analyzer should behave like this:

```c
void visitXXXNode(/* omitted for simplicity */) {
    /*
     * Step 1. Push a new symbol table if this node forms a scope.
     * Step 2. Insert the symbol into current symbol table if this node is related to
     *    declaration.
     * Step 3. Traverse child nodes of this node.
     * Step 4. Perform semantic analyses of this node.
     * Step 5. Pop the symbol table pushed at the 1st step.
     */
}
```

> [!note]
> Once your parser has found a semantic error in a child node of some node, the parser has no need to check semantic errors related to the child node since the parser cannot use the wrong information for the rest examinations.

Let's look at a simple example:

```pascal
example;

begin // 1.
    var a: 55146; // 2.
    var a: integer; // 3.
    var b: "SSLAB"; // 4.

    a := 26980; // 5.
    b := 26980 + "team"; // 6.
end
end
```

In this example, I'll skip the `ProgramNode` and directly demonstrate how should your analyzer do when meeting a `CompoundStatementNode`.

1. A `CompoundStatementNode` forms a scope. The analyzer should push a new symbol table as the current symbol table (Step 1). Then, it skips Step 2 since this node is not related to the declaration.

2. Next, the analyzer traverses the child nodes of this node, starting from the variable declaration `var a: 55146;`. When meeting the node of the variable declaration, it also performs the tasks described in `visitXXXNode()`. In Step 2, it inserts the symbol `a` into the current symbol table as a constant variable. (Omit description of other steps for simplicity.)

3. After traversing the variable declaration `var a: 55146;`, the analyzer traverse the next variable declaration `var a: integer;`. When it tries to insert the symbol `a` into the current symbol table, it finds that it's a symbol redeclaration! Therefore, it reports the error to the `stderr` and moves on to the next step. <br>
   **This is why the symbol redeclared is classified into pre-order, the semantic error of it is found and reported before visiting child nodes (Step 3).** (Omit description of the rest steps for simplicity.)

4. Omit the description of traversing the variable declaration `var b: "SSLAB";` since it's similar to the previous two.

5. The analyzer meets the assignment node `a := 26980;`. Again, it performs the tasks described in `visitXXXNode()`:
   1. Skip since assignment node doesn't form a scope.
   2. Skip since assignment node doesn't do declaration.
   3. Traverse child nodes (variable reference and expression)
      1. Traverse variable reference:
         1. Omit steps 1, 2, 3 for simplicity.
         2. In step 4, the semantic analyses of a `VariableReferenceNode` basically check whether this variable is in visible symbol tables. (More checks are described in [Section "0x02. Semantic Definition"](#0x02-semantic-definition)). It will do a lookup and find that symbol `a` is a constant variable declared in `2.`. Everything is fine, move on!
      2. Traverse expression:
         1. Omit steps 1, 2, 3 for simplicity.
         2. In step 4, since there are no semantic analyses related to a `ConstantValueNode`, just move on.
   4. Perform semantic analyses of an `AssignmentNode`, which basically check whether the variable reference is a constant or not and whether the variable reference and the expression are the same types. <br>
      The analyzer will find that the variable reference is a reference to a constant variable, and we cannot assign a value to a constant variable. As a result, it'll report the semantic error and skip the rest checks of the `AssignmentNode`.
6. Let's see a more complicated example. Also, the analyzer performs the same tasks on the assignment node `b := 26980 + "team";`:
   1. Omit steps 1, 2 for simplicity.
   2. Traverse child nodes (variable reference and expression)
      1. Traverse variable reference:
         1. Here is basically the same as the process described in 5-iii-a.
      2. Traverse expression:
         1. Omit steps 1, 2 for simplicity.
         2. Traverse child nodes (constant value and constant value)
            1. Nothing special. Omitted.
         3. In step 4, the semantic analyses of a `BinaryOperatorNode` basically check whether the operands are the same types. <br>
            The analyzer will find that the left operand is an integer, while the right operand is a string. We cannot add an integer with a string, so it will report the semantic error and skip the rest checks of the `BinaryOperatorNode`.
   3. Perform semantic analyses of an `AssignmentNode`.
      - Note that the spec has mentioned that we don't have to do further checks if the current node's child nodes have semantic errors. Be careful! The spec says that we don't have to do further checks **that are related to the child node which has a semantic error rather than all checks in the current node.** <br>
        Therefore, the analyzer will still find that the variable reference is a reference to a constant variable, which is prohibited and report the semantic error! After all, the variable reference is unrelated to the expression. We can still check the semantic definitions of the variable reference in the `AssignmentNode`.

More details about semantic definitions of each kind of node are listed in the next section.

### 0x02. Semantic Definition

This section describes the semantic definitions of each kind of node. Each kind of node has several rules to conform. Each rule is check by the order presented.

When your parser encounters a semantic error, it should report an error with relevant error messages and formatting. The semantic errors have been defined for you, encapsulated as several concrete classes in [sema/Error.hpp](src/include/sema/Error.hpp). You only need to pass the necessary information to construct the appropriate error class and send it to the [ErrorPrinter](src/include/sema/ErrorPrinter.hpp) to print the error message. The remaining semantic checks of the node should then be skipped, with the [function declaration](#function-declaration) being the only exception.

For example, to report a symbol redeclaration error, you can use the following code snippet:

```c
m_error_printer.print(SymbolRedeclarationError(node.getLocation(), node.getNameCString()));
```

You may implement your own error reporting mechanism, but you should follow the format of the error messages described in [error-message.md](/error-message.md).

> [!note]
>
> - Your parser should report the error to **`stderr`**.
> - The maximum lines of code is 200.

#### Symbol Table

The symbol redeclaration error has two patterns:

1. Normal symbol redeclaration: redeclare a symbol in the same scope.
2. Loop variable redeclaration: loop variable cannot be redeclared in the same or any inner scope

Scope rules are described in ["Scope Rule"](#scope-rules).

#### Function Declaration

If the function name causes a redeclaration error, the remaining semantic checks should still be performed.

#### Variable Declaration

(Skip the rest of semantic checks if there are any symbol redeclaration errors)

In an array declaration, each dimension's size has to be greater than 0.

#### Variable Reference

1. The identifier has to be in symbol tables.
2. The kind of symbol has to be a parameter, variable, loop_var, or constant.

(Skip the rest of semantic checks if there are any errors in the node of the declaration of the referred symbol)

3. Each index of an array reference must be of the integer type.
   - Once an incorrect (not an integer type) or unknown (an error has occurred in the child node) index was found, further checking regarding current array reference is unnecessary.
   - Bound checking is not performed at compile time as in C language.
4. An over array subscript is forbidden, that is, the number of indices of an array reference cannot be greater than the one of dimensions in the declaration.

#### Binary/Unary operator

Arithmetic operator (`+`, `-`, `*`, or `/`)

(Skip the rest of semantic checks if there are any errors in the nodes of operands)

1. Operands must be an integer or real type.
   - The types of operands can be different after appropriate type coercion.
   - The operation produces an integer or a real value.

---

Neg operator (`-`)

(Skip the rest of semantic checks if there are any errors in the node of operand)

1. The operand must be an integer or real type.
   - The operation produces an integer or a real value.

---

Mod operator (`mod`)

(Skip the rest of semantic checks if there are any errors in the nodes of operands)

1. Operands must be an integer type.
   - The operation produces an integer value.

---

Boolean operator (`and`, `or`, or `not`)

(Skip the rest of semantic checks if there are any errors in the nodes of operands)

1. Operands must be boolean type.
   - The operation produces a boolean value.

---

Relational operator (`<`, `<=`, `=`, `>=`, `>`, or `<>`)

(Skip the rest of semantic checks if there are any errors in the nodes of operands)

1. Operands must be an integer or real type.
   - The types of operands can be different after appropriate type coercion.
   - The operation produces a boolean value.

---

String concatenation (`+`)

(Skip the rest of semantic checks if there are any errors in the nodes of operands)

1. Operands must be a string type.
   - The operation produces a string value.

#### Type Coercion and Comparison

1. Integer type can be implicitly converted into the real type in several situations: assignment, argument passing, arithmetic operation, relational operation, or return statement.
2. The result of an arithmetic operation will be real type if one of the operands is real type. For example, `1.2 + 1` produces a real value.
3. Two arrays are considered to be the same type if they have the same number of elements and the same type of the element.
   - More specifically, the following attributes have to be the same:
     - type of element
     - number of dimensions
     - size of each dimension
   - Type coercion is not permitted.

#### Function Invocation

Terms in use:

- Argument (actual parameter, the expression passed in)
- Parameter (formal parameter, the variable declared in the function prototype)

---

1. The identifier has to be in symbol tables.
2. The kind of symbol has to be function.
3. The number of arguments must be the same as one of the parameters.
4. Traverse arguments:
   <br>(Skip the rest of semantic checks if there are any errors in the node of the expression (argument))
   1. The type of the result of the expression (argument) must be the same type of the corresponding parameter after appropriate type coercion.

#### Print and Read Statement

Print statement

(Skip the rest of semantic checks if there are any errors in the node of the expression (target))

1. The type of the expression (target) must be scalar type.

---

Read statement

(Skip the rest of semantic checks if there are any errors in the node of the variable reference)

1. The type of the variable reference must be scalar type.
2. The kind of symbol of the variable reference cannot be constant or loop_var.

#### Assignment

(Skip the rest of semantic checks if there are any errors in the node of the variable reference (lvalue))

1. The type of the result of the variable reference cannot be an array type.
2. The variable reference cannot be a reference to a constant variable.
3. The variable reference cannot be a reference to a loop variable, unless the current context is for loop_var initialization.
<details><summary>(Expand to see a detailed example for assigning to a loop_var)</summary>

```pascal
for i := 0 to 1 do  // loop A
begin
    for i := 10 to 11 do // loop B
    begin
        i := 20;
    end
    end do
end
end do
```

In loop B, the following steps should happen:
1. Enters a 'loop_var initialization' scope.
2. Declare loop_var `i`. Since there's already a loop_var `i` in the symbol table, it will be a redeclaration error.
3. Try to assign 10 to loop_var `i`. (Since the loop B declaration failed, the symbol `i` is from loop A).
4. Do not report an `assign to loop_var` error, because the current scope is a 'loop_var initialization' scope.
5. Enters 'loop body' scope (An ordinary scope generated by the compound statement representing loop body).
6. Try to assign 20 to loop_var `i`. (The symbol `i` is from loop A).
7. Report an `assign to loop_var` error because the current scope is an ordinary scope generated by a compound statement (loop body).
8. Leaves 'loop body' scope.
9. Leaves 'loop_var initialization' scope.

The rule aims to clarify error messages by only report redeclaration errors for a redeclared loop_var.
</details>

(Skip the rest of semantic checks if there are any errors in the node of the expression)

4. The type of the result of the expression cannot be an array type.
5. The type of the variable reference (lvalue) must be the same as the one of the expression after appropriate type coercion.

#### If and While

(Skip the rest of semantic checks if there are any errors in the node of the expression (condition))

1. The type of the result of the expression (condition) must be boolean type.

#### For

The initial value of the loop variable (lower bound) and the constant value of the condition (upper bound) must be in the _strictly_ incremental order.

#### Return

1. The current context shouldn't be in the program or a procedure since their return type is void.

(Skip the rest of semantic checks if there are any errors in the node of the expression (return value))

2. The type of the result of the expression (return value) must be the same type as the return type of current function after appropriate type coercion.

## Implementation Hints

### Recommended Workflow of Developing

1. Design and implement the symbol table construction and related checks (symbol redeclaration).
2. Add the switch button of pseudocomment D for turning on/off the symbol table dumping.
3. Implement one of semantic checks in the order listed in [0x02. Semantic Definition](#0x02-semantic-definition).
4. Examine the functionality of checks by utilizing the test cases we provide.
5. Repeat 3 ~ 4. until all semantic checks have been done.

### Symbol Table Construction

Since we need to **push** a symbol table when entering a new scope and **pop** a symbol table when exiting a scope, the property of stack - LIFO is really suitable. As a result, you may have a code snippet like this:

```cpp
class SymbolEntry {
private:
    // Variable names
    // Kind
    // Level
    // ...
};

class SymbolTable {
public:
    void addSymbol(/* attributes needed by a SymbolEntry */);
    // other methods
private:
    // keep the order by simply using vector
    std::vector<SymbolEntry> entries;
};

class SymbolManager {
public:
    void pushScope(SymbolTable *new_scope);
    void popScope();
    // other methods
private:
    std::stack<SymbolTable *> tables;
};
```

### Type Information Propagation

Some semantic checks are related to type incompatibility. You need to design a mechanism to propagate the type information from a child node to its parent node. For example, you need to propagate the type of the result of an expression to a BinaryOperatorNode, so that it can check whether its operation is legal or not.

You may reuse the class/struct used in hw3 for representing the type of P language and then store it in the AST nodes for propagating the type information.

### Error Report

As long as you report error messages to `stderr` and other messages to `stdout`, there's no need to store the error messages as a string and report them all at once. Our test script writes the messages to the file in the order of `stdout` before `stderr`, so it's fine to interleave them.

## What Should Your Parser Do?

If the input file is syntactically and semantically correct, output the following message.

```text
|---------------------------------------------------|
|  There is no syntactic error and semantic error!  |
|---------------------------------------------------|
```

Once the parser encounters a semantic error, output the related error message.

Notice that semantic errors should **not** cause the parser to stop its execution. You should let the parser keep working on finding semantic errors as much as possible.

## Project Structure

- `README.md`
- /src
  - Makefile
  - `scanner.l`
  - `parser.y`
  - /include
    - /AST
    - /sema
      - SemanticAnalyzer.hpp - for semantic analysis in visitor pattern version
    - /visitor
  - /lib
    - /AST
    - /sema
      - SemanticAnalyzer.cpp - for semantic analysis in visitor pattern version
    - /visitor
  - Other modules you may add
- /report
  - `README.md`

In this assignment, you have to do the following tasks:

- Revise `scanner.l`, `parser.y`, and add some modules (e.g., `SymbolTable.[hc]pp`, `SemanticAnalyzer.[hc]pp`) to perform a semantic analysis.
- Write a report in `report/README.md`. The report should at least describe the changes you have made in `parser.y` and the abilities of your AST.

If you want to preview your report in GitHub style markdown before pushing to GitHub, [`grip`](https://github.com/joeyespo/grip) might be the tool you need.

## Assessment Rubrics (Grading)

Total of 105 points

- Passing all test cases (95 pts)
  - hidden test cases (47.5 pts)
- Report (10 pts) \
  0: empty \
  3: bad \
  5: normal \
  7: good \
  10: excellent

## Build and Execute

- Get Hw4 docker image: `make docker-pull`
- Activate docker environment: `./activate_docker.sh`
- Build: `make`
- Execute: `./parser [input file]`
- Test: `make test`

### Build Project

TA would use `src/Makefile` to build your project by simply typing `make clean && make` from the docker container. You have to make sure that it will generate an executable named '`parser`'. **No further grading will be made if the `make` process fails or the executable '`parser`' is not found.**

### Test your parser

We provide all the test cases in the `test` folder. Simply type `make test` to test your parser. The grade you got will be shown on the terminal. You can also check `diff.txt` in `test/result` folder to know the `diff` result between the outputs of your parser and the sample solutions.

Please use `student_` as the prefix of your own tests to prevent TAs from overwriting your files. For example: `student_identifier_test`.

## Submitting the Assignment

You should push all your commits to the designated repository (hw4-\<Name of your GitHub account\>) under the GitHub organization by the deadline (given in the very beginning of this assignment description).
At any point, you may save your work and push commits to your repository. You **must** commit your final version to the **main branch**, and we will grade the commit which is last pushed on your main branch. The **push time** of that commit will be your submission time, so you should not push any commits to the main branch after the deadline if you have finished your assignment; otherwise, you will get a late penalty.

> [!note]
> The penalty for late homework is **15% per day** (weekends count as 1 day). The late submission duration is 3 days. Any commits made after this duration will not be graded.

In addition, homework assignments **must be individual work**. If we detect what we consider to be intentional plagiarism in any assignment, the assignment will receive reduced or, usually, **zero credit**.
