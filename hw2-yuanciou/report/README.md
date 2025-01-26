# hw2 report

|||
|-:|:-|
|Name|邱振源|
|ID|111550100|

## How much time did you spend on this project

> About 7-8 hours.

## Project overview

> Please describe the structure of your code and the ideas behind your implementation in an organized way. \
> The point is to show us how you deal with the problems. It is not necessary to write a lot of words or paste all of your code here.

### Tokens

Add the ```return Token``` statement to each token in the provided *scanner.l* file, for example ```":="  { listToken(":=");  return ASSIGN; }```. The token table is as follows:

|  Delimiter  |     Token       |   Operator   |    Token  |   Operator   |    Token  |
|:-----------:|:---------------:|:------------:|:---------:|:------------:|:---------:|
|   ","       |       ','       |     "+"      |     '+'   |      "<="    |     LE    |
|   ";"       |       ';'       |     "-"      |     '-'   |      "<>"    |     NE    |
|   ":"       |      ':'        |     "*"      |     '*'   |      ">"     |     '>'   |
|  "("        |       '('       |       "/"    |     '/'   |      ">="    |     GE    |
|  ")"        |       ')'       |     "mod"    |     MOD   |      "="     |     '='   |
|  "["        |       '['       |      ":="    |   ASSIGN  |      "and"   |     AND   |
|  "]"        |       ']'       |      "<"     |     '<'   |      "or"    |     OR    |
|             |                 |              |           |      "not"   |     NOT   |

| Reserved Word |     Token       | Reserved Word |     Token       |Others                       | Token   |
|:-------------:|:---------------:|:-------------:|:---------------:|:----------------------------:|:-------:|
| "var"         | KWvar           | "begin"       | KWbegin         |identifier                   | ID      |
| "array"       | KWarray         | "end"         | KWend           |string                       | Str     |
| "of"          | KWof            | "while"       | KWwhile         |integer                      | Int     |
| "boolean"     | KWboolean       | "do"          | KWdo            |0[0-7]+                      | Oct_int |
| "integer"     | KWinteger       | "if"          | KWif            |float                        | Float   |
| "real"        | KWreal          | "then"        | KWthen          |sci. notation          | Sci     |
| "string"      | KWstring        | "else"        | KWelse          |
| "true"        | KWtrue          | "for"         | KWfor           |
| "false"       | KWfalse         | "to"          | KWto            |
| "def"         | KWdef           | "print"       | KWprint         |
| "return"      | KWreturn        | "read"        | KWread          |

### Declarations (line 16-34)

I declare some tokens in the *parser.y* initially, including operators, reserved words, and others listed in the token table above, for example ```%token MOD ASSIGN LE NE GE AND OR NOT```.

Next, I defined the associativity as follows. From top to bottom, the associativity progresses from low to high precedence.
```
%left AND OR NOT
%left '<' LE GE NE '>' '='
%left '-'
%left '+'
%left '/' MOD
%left '*'
```

### Grammer Rules (line 38-138)

I usually write the grammar directly as per the assignment specification. If I happen to write the grammar with a different approach, I describe it below.

#### Program

```ProgramName: ID ';' Declaration Function Compound_S KWend;```

#### Function

I first define the *Function_Header*, then use *Function_Header* to define *Function_Declaration* and *Function_Definition*. 

And I use ```Function:|Function_Declaration Function|Function_Definition Function;``` to handle the part which it can be reduce recursively.

#### Formal Argument

I first define *Formal_Argument*, then to handle the part which could have multiple formal argument in ```Function_Header: identifier ( [ formal_argument { ; formal_argument } ] )``` I use the following grammer.
```
Formal_Arguments:
                | Formal_Argument ';' Formal_Arguments
                | Formal_Argument;
```

#### Declarations (Variable Declaration, Constant Declaration, Identifier List)

For *Constant_Declaration*, since there is a hint: *"The literal_constant cannot be applied here because it does not contain an optional negative sign."*, I think it as it apply *literal_constant* and then handle the negative sign. So, I write the below grammer.
```
Constant_Declaration: KWvar Id_List ':' Literal_Constant ';'
                    | KWvar Id_List ':' '-' Integer_Literal ';'
                    | KWvar Id_List ':' '-' Real_Literal ';';
```

And I use the following grammer to deal with the part in *Compound_Statement* which could have multiple declaration.

```
Declaration:
           | Variable_Declaration Declaration
           | Constant_Declaration Declaration;
```

#### Types (Scalar Type, Array Type)

Since there are for types of scalar type, I write the below grammer.

```Scalar_Type: KWinteger | KWreal | KWstring | KWboolean;```

And I use the following grammer to handle the part representing multidimensional array.

```
Array_Type: KWarray Integer_Literal KWof Scalar_Type 
          | KWarray Integer_Literal KWof Array_Type;
```

Finally, I use ```Type: Scalar_Type | Array_Type;```, since there are to type of *Types*.

#### Statements

I first use the below grammer to deal with *Compound_Statement* later.
```
Statement: | Statement Simple_S | Statement Conditional_S | Statement Function_Call_S 
           | Statement Loop_S | Statement Return_S | Statement Compound_S;
```

And, for *Compound_Statement* the grammer ```Compound_S: KWbegin Declaration Statement KWend;``` is correct since I defince *Declaration* could be zero or more and also the *Statement* before.

#### Expressions

For *Variable Reference*, I use the following grammer to hadle the correct *Variable_Reference* with *Array_Reference*.
```
Variable_Reference: ID | Array_Reference;
Array_Reference: ID '[' Expression ']' | Array_Reference '[' Expression ']';
```

And I use ```%prec``` to write the below grammer if there is the unary operator.

```
Unary_Operation: '-' Expression %prec '*'
               | NOT Expression;
```

## What is the hardest you think in this project

1. At first, I wasn't very familiar with the representation of "{}" and "[]" in the README, so I kept writing incorrect grammar. However, after rereading the slides, I also learned about this notation, and my writing of other grammar rules became smoother.
2. Because ambiguous grammar can be written in yacc, I usually intuitively derive the grammar when devising it. However, sometimes the order in which the grammar is reduced can affect the result. Additionally, I occasionally overlook some important details, resulting in the need to spend a lot of time debugging. Therefore, paying attention to small details and deriving the correct grammar are what I consider to be the hardest part of this assignment.

## Feedback to T.A.s

The tasks required for completing Assignment 2 are clearly written in the README file. By reading it, one can easily understand the steps needed to complete the assignment. Thank you for putting a lot of effort into designing the assignments.