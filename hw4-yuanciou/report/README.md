# hw4 report

|||
|-:|:-|
|Name|邱振源|
|ID|111550100|

## How much time did you spend on this project

> e.g. 35 hours.

## Project overview

### Changes in ```scanner.l```
To finish the part of the pseudocomment ```D```, I add ```int32_t opt_dump``` to decide whether we should dump the contents of the symbol table.

Also, I modify the following code to handle ```&D+``` and ```&D-```:
```
"//&"[STD][+-].* {
    ...
    switch (option) {
    ...
    case 'D':
        opt_dump = (yytext[4] == '+') ? 1 : 0;
        break;
    }
}
```

### Changes in ```parser.y```
In order to deal with not printind *"There is no syntactic error and semantic error!"* if there's any semantic error, I ```extern bool error_exist```, which is also extern in *ErrorPrinter.hpp*. Once the error printer need to print the error message, ```error_exist``` will become true so that "There is no syntactic error and semantic error!" will not print since the following condition code:
```
if(error_exist == false)
    {
        printf("\n"
           "|---------------------------------------------------|\n"
           "|  There is no syntactic error and semantic error!  |\n"
           "|---------------------------------------------------|\n");
    }
```
### Abilities of AST
When creating the symbol table, pass in the items to be printed along with the PType of the variable. Add the necessary functions for retrieving values in SymbolEntry, SymbolTable, and SymbolManager. Use a vector called ```cur_root``` to record the order of node visits (push when entering a node and pop when leaving a node). For certain types that need to store specific node data, there will be a manager for that node.

When entering a node, first handle any errors that need to be checked, then add items to the table according to the order in the ```/*TODO:*/```, visit the child nodes, and check for potential errors in that node. Finally, pop the table.

Take ```ForNode``` for example, since we need to know the start value and end value for a for loop, we need to record the values and determine whether the start value is bigger than end value, which will cause error:
```
class ForManager{
public:
    int loop_start;
    int loop_end;
    bool loop_error() { return ((loop_start > loop_end) ? true : false); }
};
```
Then we can follow the step above:
```
void SemanticAnalyzer::visit(ForNode &p_for) {
    /*
     * TODO:
     *
     * 1. Push a new symbol table if this node forms a scope.
     * 2. Insert the symbol into current symbol table if this node is related to
     *    declaration (ProgramNode, VariableNode, FunctionNode).
     * 3. Traverse child nodes of this node.
     * 4. Perform semantic analyses of this node.
     * 5. Pop the symbol table pushed at the 1st step.
     */
     
    SymbolTable *cur_table = new SymbolTable(symbol_manager.getLevel());
	symbol_manager.pushScope(cur_table);
	symbol_manager.cur_root.push_back("for");
	
    p_for.visitChildNodes(*this);

	symbol_manager.cur_root.pop_back();

    if(for_manager.loop_error() == true)
    {
        m_error_printer.print(NonIncrementalLoopVariableError(p_for.getLocation()));
    }

    if(opt_dump) 
    {
        dumpSymbol(cur_table);
    }

    symbol_manager.popScope();
}
```
By following the step like this, we can do semantic analyze via visting the AST to dump the tables and error messages.

## What is the hardest you think in this project

I find the entire assignment quite challenging because I am not very familiar with object-oriented syntax. This makes it easy to encounter bugs when maintaining the Symbol Table. Identifying the parts that might cause issues in the Symbol Table maintenance and eliminating these errors is, in my opinion, the most difficult part.

## Feedback to T.A.s

At first, thinking about handling so many types of errors made me feel a bit lost. However, I later discovered that the TA had already completed the files for Error and ErrorPrinter. Although sometimes I don't know how to properly insert the expected types into the functions, if I really can't figure it out, I can just modify some parts of the type and apply them directly. Having this error framework saves me a lot of time in dealing with the output for each type of error (compared to other parts of the assignment). Thank you, TA.
