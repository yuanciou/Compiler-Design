[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-22041afd0340ce965d47ae6ef1cefeee28c7c493a6346c4f15d667ab976d596c.svg)](https://classroom.github.com/a/X203vvC4)
# Project Assignment 2 - Syntactic Definitions

**Introduction to Compiler Design by Prof. Yi-Ping You**

### Due date: **23:59, November 1, 2024**

The course project has been divided into five incremental parts:

1. Implement a scanner (lexical analyzer) using `lex` (`flex`).
2. Implement a parser (syntax analyzer) using `yacc` (`bison`). (this assignment)
3. Augment the parser with the appropriate semantic actions to generate an **abstract syntax tree** (**AST**).
4. Augment the parser with appropriate semantic actions to perform a semantic analysis against the AST.
5. Augment the parser with appropriate semantic actions to generate RISC-V assembly code against the AST.

As you can see, each assignment is based on the previous one. Make sure the program you write is well-structured.

> [!note]
> You can use the reference solutions from previous assignments for later assignments, or you can replace them with your own solutions.

---

## Table of Contents

- [Project Assignment 2 - Syntactic Definitions](#project-assignment-2---syntactic-definitions)
  - [Introduction](#introduction)
  - [Syntactic Definitions](#syntactic-definitions)
    - [Program Units](#program-units)
      - [Program](#program)
      - [Function](#function)
        - [Formal Argument](#formal-argument)
    - [Declarations](#declarations)
      - [Variable Declaration](#variable-declaration)
      - [Constant Declaration](#constant-declaration)
        - [Identifier List](#identifier-list)
    - [Types](#types)
      - [Scalar Type](#scalar-type)
      - [Array Type](#array-type)
    - [Statements](#statements)
      - [Simple Statement](#simple-statement)
      - [Conditional Statement](#conditional-statement)
      - [Function Call Statement](#function-call-statement)
      - [Loop Statement](#loop-statement)
      - [Return Statement](#return-statement)
      - [Compound Statement](#compound-statement)
    - [Expressions](#expressions)
      - [Literal Constant](#literal-constant)
      - [Variable Reference](#variable-reference)
      - [Function Call](#function-call)
      - [Binary Operation](#binary-operation)
      - [Unary Operation](#unary-operation)
      - [Operator Precedence](#operator-precedence)
  - [Implementation Notes](#implementation-notes)
  - [What Should Your Parser Do?](#what-should-your-parser-do)
  - [Project Structure](#project-structure)
  - [Assessment Rubrics (Grading)](#assessment-rubrics-grading)
  - [Build and Execute](#build-and-execute)
    - [Build project](#build-project)
    - [Test your parser](#test-your-parser)
  - [Submitting the Assignment](#submitting-the-assignment)

---

## Introduction

In this assignment, you have to write an **LALR(1)** parser for the **`P`** language using `yacc`. This document provides the syntactic definitions of the `P` language. You should follow those syntactic definitions to implement the parser.

Once the LALR(1) grammar is defined, you can use `yacc` (or `bison`) to produce a C program, which is called `y.tab.c` (or `<input file>.tab.c`) by default. That program contains functions to perform the tasks defined by the LALR(1) grammar you wrote.

In `y.tab.c` (or `<input file>.tab.c`), `yyparse()` is the entry function of the parser. You are supposed to provide a `main` function that invokes `yyparse()` in your yacc source file, which has been done in the starter code. Since `yyparse()` calls `yylex()` to get a token scanned, you also have to modify your lex source file so that the scanner (`yylex()`) returns a token.

For more detailed information, please consult the documentation available in the _Private_ repository.

## Syntactic Definitions

In the this definition, the following syntactic notation is used:

- [ item ] indicates that the item is optional
- { item } denotes zero or more occurrences of the item

Terminals are presented in **boldface**, while non-terminals are denoted in _italics_. A combination of links is used for cross-referencing.

> [!note]
> A pair of square braces or curly braces in bold are simply the terminals.

The precise definition of the grammar is consistently presented in the form of a bullet-point list.

<!--
Asterisks are employed for boldface, and underscores are employed for italics. While it is possible to use the same character for both cases, opting for different characters enhances clarity and facilitates quick identification at first glance.
-->

<!--
1. For cross-referencing, add a link to the appearance in the grammar rule first. If such a non-terminal is only referenced in the description, add a link to its appearance in the description instead.
2. A non-terminal is referenced only once within a subsection.
-->

### Program Units

A program unit can be either a _[program](#program)_ or a _[function](#function)_.

<div align="right">
    <b><a href="#table-of-contents">↥</a></b>
</div>

#### Program

Followed by the name, a program consists of a sequence of _declarations_ and a _compound_statement_.

- **identifier** **;** \
   { _[declaration](#declarations)_ } \
   { _[function](#function)_ } \
   _[compound_statement](#compound-statement)_ \
   **end**

A program has no arguments, and hence no parentheses are present in the header.
All global _declarations_, including _functions_, have to appear before the _compound_statement_.

> [!note]
>
> - An **identifier** is a terminal defined in assignment 1. It does not indicate the word "identifier".
> - A _function_ can only be declared on the program level, thus it's not one of the _declaration_.

<div align="right">
    <b><a href="#table-of-contents">↥</a></b>
</div>

#### Function

A _function_ can be a _function_declaration_ or a _function_definition_. The name, parameter types, and the return type of a function are collectively referred to as a _function_header_.

A _function_header_ has the following forms:

- **identifier** **(** [ _[formal_argument](#formal-argument)_ { **;** _formal_argument_ } ] **)** **:** _[scalar_type](#scalar-type)_
- **identifier** **(** [ _formal\_argument_ { **;** _formal\_argument_ } ] **)**

A _function_ takes zero or more _formal_arguments_.

> [!note]
>
> - A _function_ that returns no value can be called as a "procedure".
> - The _scalar_type_ is not the only _[type](#types)_. There's another _[array_type](#array-type)_. However, we do not have a _function_ return an _array_type_ for simplicity.
> - The parentheses (**(** **)**) after **identifier** are required even if no _formal_argument_ is declared.

A _function_declaration_ is a _function_header_ followed by a semicolon:

-  _function_header_ **;**

A _function_definition_ is a _function_header_, followed by its body, and then ends with an **end**:

- _function_header_ \
   _[compound_statement](#compound-statement)_ \
   **end**

The following examples are all valid _function_declarations_:

```pascal
    func1(x, y: integer; z: string): boolean;
    func2(a: boolean): string;
    func3();        // procedure
    func4(b: real); // procedure
    func5(): integer;
```

<div align="right">
    <b><a href="#table-of-contents">↥</a></b>
</div>

##### Formal Argument

A _formal_argument_ has the following form:

- _[identifier_list](#identifier-list)_ **:** _[type](#types)_

The colon is followed by exactly one _type_ (including _[array_type](#array-type)_) specification.

<div align="right">
    <b><a href="#table-of-contents">↥</a></b>
</div>

### Declarations

A declaration has the following forms:

- _[variable_declaration](#variable-declaration)_
- _[constant_declaration](#constant-declaration)_

<div align="right">
    <b><a href="#table-of-contents">↥</a></b>
</div>

#### Variable Declaration

A _variable_declaration_ has the following form:

- **var** _[identifier_list](#identifier-list)_ **:** _[type](#types)_ **;**

Examples:

```pascal
var i, j: integer;
var b: boolean;
```

<div align="right">
    <b><a href="#table-of-contents">↥</a></b>
</div>

#### Constant Declaration

A _constant_declaration_ has the following form:

- **var** _[identifier_list](#identifier-list)_ **:** [ **-** ] _[integer_literal](#integer-literal)_ **;**
- **var** _[identifier_list](#identifier-list)_ **:** [ **-** ] _[real_literal](#real-literal)_ **;**
- **var** _[identifier_list](#identifier-list)_ **:** _[string_literal](#string-literal)_ **;**
- **var** _[identifier_list](#identifier-list)_ **:** _[boolean_literal](#boolean-literal)_ **;**

Examples:

```pascal
var c, d: 3;
var s: "Hello";
var pi: 3.14159;
```

> [!note]
> The _[literal_constant](#literal-constant)_ cannot be applied here because it does not contain an optional negative sign.

<div align="right">
    <b><a href="#table-of-contents">↥</a></b>
</div>

##### Identifier List

An _identifier_list_ is a list of **identifiers** separated by commas:

- **identifier** { **,** **identifier** }

The _identifier_list_ contains at least one **identifier**.

<div align="right">
    <b><a href="#table-of-contents">↥</a></b>
</div>

### Types

A _type_ has the following forms:

- _[scalar_type](#scalar-type)_
- _[array_type](#array-type)_

<div align="right">
    <b><a href="#table-of-contents">↥</a></b>
</div>

#### Scalar Type

A _scalar_type_ has the following forms:

- **integer**
- **real**
- **string**
- **boolean**

<div align="right">
    <b><a href="#table-of-contents">↥</a></b>
</div>

#### Array Type

An _array_type_ has the following form:

- **array** _[integer_literal](#integer-literal)_ **of** _[type](#types)_

The _integer_literal_ should be a non-negative integer constant that represents the size of the array.

> [!note]
> The _type_ may be expanded recursively, representing a multidimensional array. Consider the following declaration of a 2-dimensional array:
>
> ```pascal
> var L2darray: array 4 of array 2 of integer;
> ```
>
> `array 4 of array 2 of integer` has the _type_ of `array 2 of integer`, yet `array 2 of integer` recursively has the _type_ of `integer`.

<div align="right">
    <b><a href="#table-of-contents">↥</a></b>
</div>

### Statements

A _statement_ can be one of the following statements:

- _[simple_statement](#simple-statement)_
- _[conditional_statement](#conditional-statement)_
- _[function_call_statement](#function-call-statement)_
- _[loop_statement](#loop-statement)_
- _[return_statement](#return-statement)_
- _[compound_statement](#compound-statement)_

<div align="right">
    <b><a href="#table-of-contents">↥</a></b>
</div>

#### Simple Statement

A _simple_statement_ can be an _assignment_, a _print_statement_, or a _read_statement_.

An _assignment_ has the following form:

- _[variable_reference](#variable-reference)_ **:=** _[expression](#expressions)_ **;**

A _print_statement_ has the following form:

- **print** _expression_ **;**

<div align="right">
    <b><a href="#table-of-contents">↥</a></b>
</div>

A _read_statement_ has the following form:

- **read** _variable_reference_ **;**

<div align="right">
    <b><a href="#table-of-contents">↥</a></b>
</div>

#### Conditional Statement

A _conditional_statement_ has the following forms:

- **if** _[expression](#expressions)_ **then** \
   _[compound_statement](#compound-statement)_ \
   **else** \
   _compound_statement_ \
   **end** **if**
- **if** _expression_ **then** \
   _compound_statement_ \
   **end** **if**

Here is an example of printing the sign of the number `n`:

```pascal
if n >= 0 then
    begin
        print "+";
    end
else
    begin
        print "-";
    end
end if
```

<div align="right">
    <b><a href="#table-of-contents">↥</a></b>
</div>

#### Function Call Statement

A _function_call_statement_ is simply a _function_call_ followed by a semicolon:

- _[function_call](#function-call)_ **;**

<div align="right">
    <b><a href="#table-of-contents">↥</a></b>
</div>

#### Loop Statement

A _loop_statement_ may be a _while_statement_:

- **while** _[expression](#expressions)_ **do** \
   _[compound_statement](#compound-statement)_ \
   **end** **do**

Or a _for_statement_:

- **for** **identifier** **:=** _[integer_literal](#integer-literal)_ **to** _integer_literal_ **do** \
   _compound_statement_ \
   **end** **do**

For example, the following `for` loop prints `1 2 3`:

```pascal
for i := 1 to 4 do  // the end is excluded
begin
    print i;
end
end do
```

<div align="right">
    <b><a href="#table-of-contents">↥</a></b>
</div>

#### Return Statement

A _return_statement_ has the following form:

- **return** _[expression](#expressions)_ **;**

> [!note]
> A procedure implicitly returns at the end of its body.
> Since procedures do not return a value, early returns are not supported.

<div align="right">
    <b><a href="#table-of-contents">↥</a></b>
</div>

#### Compound Statement

A _compound_statement_ consists of a block of variable and constant declarations and a block of statements, and is delimited by the reserved words **begin** and **end**:

- **begin** \
   { _[declaration](#declarations)_ } \
   { _[statement](#statements)_ } \
   **end**

All _declarations_ have to appear before the _statements_. Since the _compound_statement_ itself is a _statement_, it can appear recursively.

Here is an example of a nested _compound_statement_:

```pascal
begin
    var radius: 3;  // a constant declaration
    var area: real; // a variable declaration
    begin
        area := calculateCircleArea(3);  // an assignment with a function call
    end
    print area;  // a print statement
end
```

<div align="right">
    <b><a href="#table-of-contents">↥</a></b>
</div>

### Expressions

An _expression_ can be one of the followings:

- _[literal_constant](#literal-constant)_
- _[variable_reference](#variable-reference)_
- _[function_call](#function-call)_
- _[binary_operation](#binary-operation)_
- _[unary_operation](#unary-operation)_
- **(** _expression_ **)**

<div align="right">
    <b><a href="#table-of-contents">↥</a></b>
</div>

#### Literal Constant

A _literal_constant_ is a constant of the proper types:

- _[integer_literal](#integer-literal)_
- _[real_literal](#real-literal)_
- _[string_literal](#string-literal)_
- _[boolean_literal](#boolean-literal)_

<div align="right">
    <b><a href="#table-of-contents">↥</a></b>
</div>

##### Integer Literal

An _integer_literal_ may be octal or decimal:

- **octal_integer**
- **decimal_integer**

<div align="right">
    <b><a href="#table-of-contents">↥</a></b>
</div>

##### Real Literal

A _real_literal_ may be a floating-point number or scientific notation:

- **floating_point_number**
- **scientific_notation**

<div align="right">
    <b><a href="#table-of-contents">↥</a></b>
</div>

##### String Literal

A _string_literal_ is simply the string constant:

- **string_constant**

<div align="right">
    <b><a href="#table-of-contents">↥</a></b>
</div>

##### Boolean Literal

A _boolean_literal_ is one of the two boolean values:

- **true**
- **false**

<div align="right">
    <b><a href="#table-of-contents">↥</a></b>
</div>

#### Variable Reference

A _variable_reference_ can be simply an **identifier** or an _array_reference_ in the form of

- **identifier** { **[** _[expression](#expressions)_ **]** }

For example, `arr[30]`, `arr[30][33]`, and `arr[1 + 2]` are all syntactically valid _array_references_.

<div align="right">
    <b><a href="#table-of-contents">↥</a></b>
</div>

#### Function Call

A _function_call_ has the following form:

- **identifier** **(** [ _[expression](#expressions)_ { **,** _expression_ } ] **)**

For example, `renderScene()`, `toFahrenheit(37.5)`, and `add(1, 2)` are all syntactically valid _function_calls_.

<div align="right">
    <b><a href="#table-of-contents">↥</a></b>
</div>

#### Binary Operation

A _binary_operation_ is written in infix notation:

- _[expression](#expressions)_ _binary_operator_ _expression_

The _binary_operators_ are **+**, **-**, **\***, **/**, **mod**, **<**, **<=**, **<>**, **>=**, **>**, **=**, **and**, and **or**, consisting of numeric, relational, and logical operators.

<div align="right">
    <b><a href="#table-of-contents">↥</a></b>
</div>

#### Unary Operation

A _unary_operation_ has the following form:

- _unary_operator_ _[expression](#expressions)_

The _unary_operators_ are **-** and **not**.

<div align="right">
    <b><a href="#table-of-contents">↥</a></b>
</div>

#### Operator Precedence

The order of precedence is among the following classes of the operators, in decreasing order of precedence:

| Precedence |                        Operator                         |      Description       |
| :--------: | :-----------------------------------------------------: | :--------------------: |
|     1      |                         **`-`**                         |    negation (unary)    |
|     2      |                         **`*`**                         |     multiplication     |
|     3      |                   **`/`**, **`mod`**                    | division and remainder |
|     4      |                         **`+`**                         |        addition        |
|     5      |                         **`-`**                         |  subtraction (binary)  |
|     6      | **`<`**, **`<=`**, **`<>`**, **`>=`**, **`>`**, **`=`** |       relational       |
|     7      |             **`and`**, **`or`**, **`not`**              |        logical         |

> [!note]
>
> - The token "**-**" can be either the binary subtraction operator or the unary negation operator.
> - The associativity is left, and parentheses may be used to group subexpressions, influencing the precedence.
> - Operator precedence will be checked in Assignment III, as this assignment employs empty actions for grammar rules, and mismatches in rule matching don't necessarily disrupt syntax matching. Consequently, there's no observable behavior to check whether the operations are parsed in the correct precedence.
> - Semantic checking will be handled in the Assignment IV. In this assignment, you don't need to check semantic errors like `a := 3 + true;`. Just take care of syntactic errors.

## Implementation Notes

- Although we make each grammar rule precise, you do not have to implement them in the same way, as long as the syntax remains unchanged.
- `yyparse()` needs to use `yylex()` to obtain a token from the input source file. Therefore, you have to modify the lex source file to make the scanner pass token information to `yyparse()`. For example, when the scanner recognizes an identifier, the action should be like:

```lex
    ([A-Za-z])([A-Za-z0-9])*		{ listLiteral("id", yytext); return ID; }
    /* Note that the symbol `ID` is defined by the yacc parser */
```

- Here is a general form of context-free grammar to describe grammar rule of "zero or more items" (e.g., a function signature contains zero or more formal arguments):

```bison
    argseq : /* empty */
           | argseq1
           ;

    argseq1 : arg
            | argseq1 ',' arg
            ;
```

- `Bison` will generate `xxx.h` when you build your yacc source file (`xxx.y`) with `-d` option. The `xxx.h` has the same purpose as `y.tab.h` mentioned in the lecture note.

## What Should Your Parser Do?

The parser should list information according to **opt_tok** and **opt_src** options (same as in Assignment I). If the input file is syntactically correct, output

```
    |--------------------------------|
    |  There is no syntactic error!  |
    |--------------------------------|
```

Once the parser finds a syntactic error, generate an error message in the form of

```
    |--------------------------------------------------------------------------
    | Error found in Line #<line number where the error occurs>: <source code of that line>
    |
    | Unmatched token: <the token that is not recognized>
    |--------------------------------------------------------------------------
```

## Project Structure

- README.md
- /src
  - Makefile
  - **`scanner.l`**
  - **`parser.y`**
- /report
  - **`README.md`**

In this project, you have to modify `src/scanner.l`, extend `src/parser.y` (provided), and write your report in `report/README.md`.

> [!note]
> You can extend your `scanner.l` from Assignment I rather than using `scanner.l` we provided.

The report should at least describe what changes you have made to your scanner and the abilities of your parser.

If you want to preview your report in GitHub style markdown before pushing to GitHub, [grip](https://github.com/joeyespo/grip) might be the tool you need.

## Assessment Rubrics (Grading)

Total of 116 points

- Passing all test cases (106 pts)
  - hidden test cases (53 pts)
- Report (10 pts) \
  0: empty \
  3: bad \
  5: normal \
  7: good \
  10: excellent

## Build and Execute

- Get HW2 docker image: `make docker-pull`
- Activate docker environment: `./activate_docker.sh`
- Build: `make`
- Execute: `./parser <input file>`
- Test: `make test`

### Build project

TA would use `src/Makefile` to build your project by simply typing `make clean && make`. You don't need to modify this file, but if you do, it is **your responsibility** to make sure this makefile have at least the same make targets we provided to you.

### Test your parser

We provide all the test cases in the `test` directory. Simply type `make test` to test your parser. The grade you got will be shown on the terminal. You can also check `diff.txt` in `test/result` directory to know the `diff` result between the outputs of your parser and the sample solutions.

Please using `student_` as the prefix of your own tests to prevent TAs from overwriting your files. For example: `student_identifier_test`.

## Submitting the Assignment

You should push all your commits to the designated repository (hw2-\<Name of your GitHub account\>) under our GitHub organization by the deadline (given in the very beginning of this assignment description). At any point, you may save your work and push the repository. You **must** commit your final version to the **main branch**, and we will grade the commit which is **last pushed** on your main branch. The **push time** of that commit will be your submission time, so you **should not** push any commits to the main branch after the deadline if you have finished your assignment; otherwise, you will get a late penalty.

> [!note]
> The penalty for late homework is **15% per day** (weekends count as 1 day). The late submission duration is 3 days. Any commits made after this duration will not be graded.

In addition, homework assignments **must be individual work**. If I detect what I consider to be intentional plagiarism in any assignment, the assignment will receive reduced or, usually, **zero credit**.
