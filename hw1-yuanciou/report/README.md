# hw1 report

|Field|Value|
|-:|:-|
|Name|邱振源|
|ID|111550100|

## How much time did you spend on this project

About 7 to 8 hours.

## Project overview

### Definition Part

**C declaration and includes  (line 1-24)**

It's provided.

**Regex (line 26-36)**

Write the regular definitions of tokens.

**States (line 38-39)**

```C_stytle_comment``` is used for the comment start with "/*"
```CPP_stytle_comment``` is used for the comment start with "//"

### Rules Part

**Delimiters (line 42-49)**

Use ```listToken()``` to print the delimiters.

**Arithmetic, Relational, and Logical Operators (line 51-66)**

Use ```listToken()``` to print the arithmetic, relational, and logical operators.

**Reserved Words (line 68-96)**

Use ```listToken()``` to print the reserved words.

**Identifier (line 98-99)**

Use ```listLiteral()``` to print the token and the string that form the token.

**Integer Constants (line 101-103)**

Use ```listLiteral()``` to print the token(integer & oct_integer) and the string that form the token.

**Floating-Point Constants (line 105-106)**

Use ```listLiteral()``` to print the token and the string that form the token.

**Scientific Notations (line 108-109)**

Use ```listLiteral()``` to print the token and the string that form the token.

**String Constants (line 111-143)**

1. Use a pointer to copy the ```yytext```.
2. Run a for loop. If there are consecutive ```"```, only print once. And, for the other condition, just simply output the current character.

**Whitespace (line 145-146)**

Not require to do anything. Use ```{}```.

**Pseudocomments (line 148-152)**

If the first five character is ```//&S-```, turn the ```opt_src``` off.
If the first five character is ```//&T-```, turn the ```opt_tok``` off.
If the first five character is ```//&S+```, turn the ```opt_src``` on.
If the first five character is ```//&T+```, turn the ```opt_tok``` on.

**Comments (line 154-164)**

1. C-style
- While the status is ```INITIAL``` or ```C_style_comment```, and facing "\n", not require to do anything.
- While ```/*``` is detected, begin the ```C_style_comment```, and wait until ```*/``` is detected.

2. C++-style
- While the status is ```CPP_style_comment```, change to ```INITIAL``` in the end.
- While ```//``` is detected, begin the ```CPP_style_comment```, and wait until the status is not ```CPP_style_comment```, i.e. change to ```INITIAL```.

**Error handling (line 167~171)**

It's provided.

### Routines Part (line 175~244)

It's provided.

## What is the hardest you think in this project

1. Understanding how to write lex took me lots of time. While writing lex is somewhat similar to writing in C or C++, there are still many differences. Trying to grasp the significance of these nuances rather than just applying them took me a considerable amount of time.
2. Writing the parts for strings and comments was challenging because these sections involve more than just analyzing individual characters. Breaking down these two categories into many small components, compared to other parts of this assignment, required a significant amount of time and effort.

## Feedback to T.A.s

The required parts were described very clearly. Thank you for the professor and TA for putting a lot of effort into designing the assignments.
