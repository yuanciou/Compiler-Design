# Project Assignment 1 - Lexical Definition

**Introduction to Compiler Design by Prof. Yi-Ping You**

### Due date: **23:59, March 21, 2024**

In this assignment, you have to write a scanner for the **`P`** language in `lex`. This document provides the lexical definitions of the `P` language. You should follow those lexical definitions to implement the scanner.

The course project has been divided into five incremental parts:

1. Implement a scanner (lexical analyzer) using `lex` (`flex`). (this assignment)
2. Implement a parser (syntax analyzer) using `yacc` (`bison`).
3. Augment the parser with the appropriate semantic actions to generate an **abstract syntax tree** (**AST**).
4. Augment the parser with appropriate semantic actions to perform a semantic analysis against the AST.
5. Augment the parser with appropriate semantic actions to generate RISC-V assembly code against the AST.

As you can see, each assignment is based on the previous one. Make sure the program you write is well-structured.

---

**Table of Contents**

- [Project Assignment 1 - Lexical Definition](#project-assignment-1---lexical-definition)
  - [Character Set](#character-set)
  - [Lexical Definition](#lexical-definition)
    - [Tokens That Will Be Passed to the Parser](#tokens-that-will-be-passed-to-the-parser)
      - [Delimiters](#delimiters)
      - [Arithmetic, Relational, and Logical Operators](#arithmetic-relational-and-logical-operators)
      - [Reserved Words](#reserved-words)
      - [Identifiers](#identifiers)
      - [Integer Constants](#integer-constants)
      - [Floating-Point Constants](#floating-point-constants)
      - [Scientific Notations](#scientific-notations)
      - [String Constants](#string-constants)
    - [Tokens That Will Be Discarded](#tokens-that-will-be-discarded)
      - [Whitespace](#whitespace)
      - [Comments](#comments)
      - [Pseudocomments](#pseudocomments)
  - [Implementation Hints](#implementation-hints)
    - [Token listing](#token-listing)
    - [Comments and Strings](#comments-and-strings)
  - [What Should Your Scanner Do](#what-should-your-scanner-do)
    - [Error Handling](#error-handling)
  - [Project Structure](#project-structure)
  - [Build and Execute](#build-and-execute)
    - [Build project](#build-project)
    - [Test your scanner](#test-your-scanner)
  - [Submitting the Assignment](#submitting-the-assignment)

---

A `P` program may resemble the following example, which demonstrates a simple implementation of Fibonacci number calculation:

```pascal
//&S-
/*
 * Prints the 10th number of the Fibonacci sequence.
 */

//&S+
FibonacciNumber;

var n: 10;

// Returns the nth Fibonacci number.
fib(n: integer): integer
begin
  var i, a, b, result: integer;

  // Base cases.
  if n < 2 then
  begin
    return n;
  end
  end if

  i := 2;  // Starts from calculating the 2nd Fibonacci number.
  a := 0;  // 0th
  b := 1;  // 1st
  while i <= n do
  begin
    result := a + b;
    a := b;
    b := result;
  end
  end do
  return result;
end
end

begin
  print fib(n);
end
end
```

The seemingly unusual comments in the code are known as [psdueocomment](#pseudocomments), which serve as options that can influence the compiler's behavior.

<details>
  <summary>Click to expand! <br>

  Note that the detailed output format will be explained in the [What Should Your Scanner Do](#what-should-your-scanner-do) section, so you can skip the example output here.</summary>

```
6: //&S+
<id: FibonacciNumber>
<;>
7: FibonacciNumber;
8:
<KWvar>
<id: n>
<:>
<integer: 10>
<;>
9: var n: 10;
10:
11: // Returns the nth Fibonacci number.
<id: fib>
<(>
<id: n>
<:>
<KWinteger>
<)>
<:>
<KWinteger>
12: fib(n: integer): integer
<KWbegin>
13: begin
<KWvar>
<id: i>
<,>
<id: a>
<,>
<id: b>
<,>
<id: result>
<:>
<KWinteger>
<;>
14:   var i, a, b, result: integer;
15:
16:   // Base cases.
<KWif>
<id: n>
<<>
<integer: 2>
<KWthen>
17:   if n < 2 then
<KWbegin>
18:   begin
<KWreturn>
<id: n>
<;>
19:     return n;
<KWend>
20:   end
<KWend>
<KWif>
21:   end if
22:
<id: i>
<:=>
<integer: 2>
<;>
23:   i := 2;  // Starts from calculating the 2nd Fibonacci number.
<id: a>
<:=>
<integer: 0>
<;>
24:   a := 0;  // 0th
<id: b>
<:=>
<integer: 1>
<;>
25:   b := 1;  // 1st
<KWwhile>
<id: i>
<<=>
<id: n>
<KWdo>
26:   while i <= n do
<KWbegin>
27:   begin
<id: result>
<:=>
<id: a>
<+>
<id: b>
<;>
28:     result := a + b;
<id: a>
<:=>
<id: b>
<;>
29:     a := b;
<id: b>
<:=>
<id: result>
<;>
30:     b := result;
<KWend>
31:   end
<KWend>
<KWdo>
32:   end do
<KWreturn>
<id: result>
<;>
33:   return result;
<KWend>
34: end
<KWend>
35: end
36:
<KWbegin>
37: begin
<KWprint>
<id: fib>
<(>
<id: n>
<)>
<;>
38:   print fib(n);
<KWend>
39: end
<KWend>
40: end
```

</details>

Now, let's explore how the scanner should be implemented. :monocle_face:

## Character Set

**`P`** programs are formed from ASCII characters. Control characters are not used in the definitions of the `P` language except **`\n`** (line feed) and **`\t`** (horizontal tab).

## Lexical Definition

Tokens are divided into two classes:

1. Tokens that will be passed to the parser.
2. Tokens that will be discarded (i.e., recognized but not passed to the parser).

### Tokens That Will Be Passed to the Parser

The following tokens are recognized by the scanner in this assignment and will be passed to the parser in the next assignment.

#### Delimiters

The following strings are treated as delimiters. Each of them should be recognized by the scanner in this assignment and passed to the parser as a token in the next assignment.

||Delimiter|
|:-:|:-:|
|comma|**`,`**|
|semicolon|**`;`**|
|colon|**`:`**|
|parentheses| **`(`**, **`)`**|
|square brackets| **`[`**, **`]`**|

#### Arithmetic, Relational, and Logical Operators

The following strings are treated as operators. Each of them should be recognized by the scanner in this assignment and passed to the parser as a token in the next assignment.

||Operator|
|:-:|:-:|
|addition|**`+`**|
|subtraction|**`-`**|
|multiplication|**`*`**|
|division| **`/`**, **`mod`**|
|assignment|**`:=`**|
|relational| **`<`**, **`<=`**, **`<>`**, **`>=`**, **`>`**, **`=`** |
|logical|**`and`**, **`or`**, **`not`**|

#### Reserved Words

Each of the following strings is treated as a reserved word of the `P` language. Notice that they are case-sensitive. Each of them should be recognized by the scanner in this assignment and passed to the parser as a token in the next assignment.

- Declaration: `var`, `def`
- Type: `array`, `of`, `boolean`, `integer`, `real`, `string`
- Value: `true`, `false`
Flow-of-control: `while`, `do`, `if`, `then`, `else`, `for`, `to`
- Block: `begin`, `end`
- Statement: `print`, `read`, `return`

> [!note]
> Although `def` is listed as one of the reserved words, it is not actually used in `P` language programs.

#### Identifiers

An identifier is a sequence of letters and digits beginning with a letter. Identifiers are case-sensitive; that is, **gura**, **Gura**, and **GURA** are treated as different identifiers. Note that reserved words CANNOT be used as identifiers.

#### Integer Constants

A sequence of one or more digits. An integer that begins with the digit 0 **and** is followed by a sequence of one or more octal digits (0 ~ 7) is treated as an **octal** integer; otherwise, the sequence of digit(s) is treated as a **decimal** integer. Notably, decimal integers cannot have leading zeros.

For example, `0` is a decimal integer because it is followed by zero octal digits. On the other hand, `019` is considered as two separate integers: an octal integer `01` and a decimal integer `9`. `09` is considered as two decimal integers, a `0` and a `9`, because it has a leading zero.

#### Floating-Point Constants

A floating-point constant is formed by an integral part, a dot (`.`), and a fractional part. The dot (`.`) symbol is used to separate the integral part and the fractional part.

The integral part is a decimal integer (see [Integer Constants](#integer-constants)) while the fractional part is a sequence of one or more digits without any redundant trailing `0`. Even a single `0` is considered redundant for any floating-point constants with a non-zero fractional part. Here are some examples:

- `009.1` is a valid input according to the lexical definition, but it is recognized as two separate tokens:
  - An octal integer: `00`
  - A floating-point: `9.1`
- `9.10` is a floating point constant `9.1`, followed by a decimal integer `0` because the trailing `0` is redundant.
- `0.0` is a valid floating-point since `0` is a decimal integer and there's no redundant trailing `0`.

#### Scientific Notations

Scientific notation is a way of writing numbers that accommodate very large or small values to be conveniently written in the decimal form. Numbers are written as **`aEb`** or **`aeb`** (**`a`** times ten to the power of **`b`**), where the coefficient **`a`** is a nonzero decimal number (either a nonzero decimal integer or a nonzero floating-point number), and the exponent **`b`** is a **decimal integer** (see [Integer Constants](#integer-constants)) prefixed with an optional sign.

For instance, `1.23E4`, `1.23E+4`, `12.3E-4`, and `123E4` are valid scientific notations, whereas `0.0e1` and `0123e1` are not valid, as they violate the requirement of having a as zero and being in octal notation, respectively. It's important to note that `0.0e1` and `0123e1` are still valid inputs, as `e1` being recognized as an identifier.

#### String Constants

A string constant is a sequence of zero or more ASCII characters wrapped by two double quotes (`"`). A string constant should not contain any embedded newline(s). A double quote can be placed within a string constant by writing two consecutive double quotes. For example, an input `"aa""bb"` denotes the string constant `aa"bb`.

> [!note]
> You may assume that a string constant is always properly closed. That is, there is no need to check for unclosed string constants.

### Tokens That Will Be Discarded

The following tokens are recognized by the scanner in this assignment, but they are discarded, rather than passed back to the parser, in the next assignment.

#### Whitespace

A sequence of blank spaces, tabs, and newlines.

#### Comments

Comments can be denoted in two ways:

- *C-style* is a text wrapped by **`/*`** and **`*/`**, and may span more than one line. Note that C-style comments **do not "nest"**. Namely, **`/*`** always closes with the first **`*/`** encountered.
- *C++-style* is a one-line text prefixed with **`//`**.

Whichever comment style encountered first remains in effect until the appropriate comment close is encountered. For example, the following two comments are both valid comments:

```
// this is a comment // line  */ /* with some /* delimiters */  before the end

/* this is a comment // line with some /* and \\
// delimiters */
```

> [!note]
> You may assume that C-style comments are always properly closed. That is, there is no need to check for unclosed comments.

#### Pseudocomments

A psdueocomment is a special form of the *C++-style* comment and is used to signal options to the scanner.

Each **`pseudocomment`** consists of a *C++-style* comment delimiter (`//`), a character **`&`**, an upper-case letter, and either a **`+`** or **`-`** (**`+`** turns the option "on" while **`-`** turns the option "off"). In other words, each **`pseudocomment`** either has the form `//&C+` or the form `//&C-` where `C` denotes the option.

There may be up to 26 different options (A-Z). Specific options will be defined in the project description. A comment that does not match the option pattern exactly has no effect on the option settings. Undefined options have no special meaning; that is, such **`pseudocomments`** are treated as regular comments.

This assignment defines two options, **`S`** and **`T`**. **`S`** turns **source program listing** on or off, and **`T`** turns **token listing** on or off. The option setting takes effect from the line where that pseudocomment is written. By default, both options are on. For example, the following comments are **pseudocomments**:

- `//&S+`
- `//&S-`
- `//&S+&S-`  ***This causes the `S` option on because the comment after the pseudocomment is ignored***

## Implementation Hints

### Token listing

There are two functions in the starter code (`src/scanner.l`): **`listToken`** and　**`listLiteral`**. You may write your scanner actions using the aforementioned functions or implement the **`token listing`** option yourself.

The meaning and the usage of the functions are as follows.

- **`listToken`**: Print out the token.
- **`listLiteral`**: Print out both the token and the string that constitutes the token.

The first argument of both functions is a string. This string names the token that will be passed to the parser in the next assignment. **`listLiteral`** takes a second argument that is also a string. Some examples are given below:

|Token|Lexeme|Function Call|
|:-:|:-:|:-|
|left parenthesis|(|`listToken("(");`|
|begin|begin|`listToken("KWbegin");`|
|identifier|ab123|`listLiteral("id","ab123");`|
|integer constant|123|`listLiteral("integer", "123");`|

### Comments and Strings

In Flex, there are two types of *Start Conditions*: `%s` and `%x`. `%s` denotes inclusive start conditions, while `%x` signifies exclusive start conditions. Using one of these start conditions can simplify your implementation. For more detailed information, please consult the documentation available in the *Private* repository.

Additionally, when recognizing strings, consider utilizing functions within the *Action* (the `{}` block). You don't need to create a single, complex regular expression to handle everything – it may not even be feasible. Instead, use the available tools and functions to streamline your implementation.

## What Should Your Scanner Do

Your scanner must print out source codes and tokens (or not) according to the **`S`**(source program listing), **`T`**(token listing) options.

- If **`S`** (listing option) is on, each line should be listed, along with a line number.
- If **`T`** (token option) is on, each token should be printed on a separate line, wrapped by angle brackets.
  - The **Reserved Words** token should be printed as **`KW<lexeme>`**.
  - The **Identifiers** token should be printed as **`id: <lexeme>`**.
  - The **Integer Constants** token should be printed as **`integer: <lexeme>`** or **`oct_integer: <lexeme>`**.
  - The **Floating-Point Constants** token should be printed as **`float: <lexeme>`**.
  - The **Scientific Notations** token should be printed as **`scientific: <lexeme>`**.
  - The **String Constants** token should be printed as **`string: <lexeme>`**.

For example, given the input:

```pascal
// print hello world
begin
  var a : integer;
  var b : real;
  print "hello world";
  a := 1 + 1;
  b := 1.23;
  if a > 01 then
    b := b * 1.23e-1;
    //&S-
    a := 1;
    //&S+
    //&T-
    a := 2;
    //&T+
  end if
end
```

Your scanner should output:

```pascal
1: // print hello world
<KWbegin>
2: begin
<KWvar>
<id: a>
<:>
<KWinteger>
<;>
3:   var a : integer;
<KWvar>
<id: b>
<:>
<KWreal>
<;>
4:   var b : real;
<KWprint>
<string: hello world>
<;>
5:   print "hello world";
<id: a>
<:=>
<integer: 1>
<+>
<integer: 1>
<;>
6:   a := 1 + 1;
<id: b>
<:=>
<float: 1.23>
<;>
7:   b := 1.23;
<KWif>
<id: a>
<>>
<oct_integer: 01>
<KWthen>
8:   if a > 01 then
<id: b>
<:=>
<id: b>
<*>
<scientific: 1.23e-1>
<;>
9:     b := b * 1.23e-1;
<id: a>
<:=>
<integer: 1>
<;>
12:     //&S+
13:     //&T-
14:     a := 2;
15:     //&T+
<KWend>
<KWif>
16:   end if
<KWend>
17: end
```

### Error Handling

If the input pattern cannot match any rules, print out the line number and the first character of that input pattern and then abort the program. The output format is as follows.

> `Error at line <line number>: bad character "<character>"`
For example,
> `Error at line 3: bad character "$"`

## Project Structure

- README.md
- /src
  - Makefile
  - **`scanner.l`**
- /report
  - **`READMD.md`**

In this project, you have to finish `src/scanner.l` and write your report in `report/READMD.md`.
The report should at least describe the abilities of your scanner.

If you want to preview your report in GitHub style markdown before pushing to GitHub, [grip](https://github.com/joeyespo/grip) might be the tool you want.

## Assessment Rubrics (Grading)

Total of 105 points

- Passing all test cases (100 pts)
  - hidden test cases (50 pts)
- Report (5 pts) \
0: empty \
3: normal \
5: good

## Build and Execute

- Get HW1 docker image: `make docker-pull`
- Activate docker environment: `./activate_docker.sh`
- Build: `make`
- Execute: `./scanner <input file>`
- Test: `make test`

### Build project

TA will use `src/Makefile` to build your project by simply typing `make clean && make`. Normally, you don't need to modify this file in this assignment, but if you do, it is **your responsibility** to make sure this makefile has at least the same make targets we provided to you.

### Test your scanner

We provide all the test cases in the `test` folder. Simply type `make test` at the root directory of your repository to test your scanner. The grade you got will be shown on the terminal. You can also check `diff.txt` in `test/result` directory to know the `diff` result between the outputs of your scanner and the sample solutions.

Please using `student_` as the prefix of your own tests to prevent TAs from overwriting your files. For example: `student_identifier_test`.

## Submitting the Assignment

You should push all your commits to the designated repository (hw1-\<Name of your GitHub account\>) under our GitHub organization by the deadline (given in the very beginning of this assignment description). At any point, you may save your work and push the repository. You **must** commit your final version to the **main branch**, and we will grade the commit which is **last pushed** on your main branch. The **push time** of that commit will be your submission time, so you **should not** push any commits to the main branch after the deadline if you have finished your assignment; otherwise, you will get a late penalty.

> [!note]
> The penalty for late homework is **15% per day** (weekends count as 1 day). The late submission duration is 3 days. Any commits made after this duration will not be graded.

In addition, homework assignments **must be individual work**. If I detect what I consider to be intentional plagiarism in any assignment, the assignment will receive reduced or, usually, **zero credit**.
