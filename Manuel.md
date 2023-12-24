# WTFITS

I was working on BC-DeciCalc, and I had some thoughts:
* Use an SQLite database for the save file format.
* Be able to view SQLite databases using the spreadsheet interface.
* Be able to change out the number system in use.

This is the result of that. Those three goals were achieved, though I really need to just rewrite it. It isn't performant.

## More notes on number systems:
mpfr uses a strange pseudo-decimal precision: decimal digits are converted to bits internally.
Both mpfr and libmpdec use the bc table for figuring out the resultant number of digits of operations:

| Operation | Scale |
| --------- | ----- |
| Addition/Subtraction | `max(a, b)` |
| Multiplication | `min(a + b, max(a, b, s))` |
| Division | `s` |


## Starting the Program

* The very first argument is one of `-0`, `-1`, `-2`, `-3`, `-4`, or `-5`. This is the number system to use.
* The next accepted argument is `-l`, which specifies a Backwards library file to load. There can be a chain of multiple libraries, however: `-l MyBetterLib.txt -l TheBaseLibrarySucks.txt`. These must be at the beginning.
* The following accepted argument is `-b`, which initiates batch mode. For each `-b` argument, the next argument is expected to be a formula to evaluate. The program will evaluate each batch command and then stop before entering interactive mode. This can be used to: use DeciCalc as a command-line calculator; query the contents of a spreadsheet from a shell script; or output the value of a cell whose contents are too large to see in interactive mode.
* The first argument after all explicit arguments is a file to load. If no file is loaded, then "untitled.wts" is used.
* The second argument is the file name of an SQLite database to analyze.
* Any other arguments are ignored.


## Navigating the Spreadsheet

Most of the navigation and commands are taken from the Unix tool `sc`. The tool `sc` was originally written with `vi` users in mind, and many choices reflect that. The color scheme, and that F7 exits, is taken from WordPerfect.

### Commands
* Arrow keys : navigate. One can also use the vi keys `hjkl`.
* Page Up / Page Down : move to the next screen of rows. One can also use `JK`.
* `H` / `L` : move to the next screen of columns.
* Home : goto cell A1
* `g` : type in a cell name, then enter, and the current cell cursor will be moved to that cell. Note that you cannot see the cell name that you are typing.
* `G` : type in a table name, then enter, and view that table (from the SQLite file to analyze) as a sheet.
* `<` : start entering a label in this cell. Finish by pressing enter. (There are no centered or right-justified labels.)
* `=` : start entering a formula in this cell. Finish by pressing enter.
* `q` or F7 : exit. You must next press either 'y' to save and exit, or 'n' to not save and exit, in order to actually exit.
* `!` : recalculate the sheet
* `W` : save the sheet
* `dd` : clear (delete) the current cell
* `dr` : clear all cells in the current row
* `dc` : clear all cells in the current column
* `yy` : copy the current cell
* `pp` : paste the current cell
* `e` : edit the current cell's contents
* Shift left/right (also F9/F12) : widen or narrow the current column. Columns can be between 1 and 40 cells wide.
* `#` : Switch between column-major and row-major recalculation.
* `$` : Switch between top-to-bottom and bottom-to-top recalculation.
* `%` : Switch between left-to-right and right-to-left recalculation.
* `,` : Toggle between using ',' and '.' as the decimal separator. This is not a saved setting.
* `+` : If the current cell is empty, start entering a formula in this cell, else enter edit mode and append to this cell. If the current cell is a formula, append a '+' to the formula.
* `:)` : Goto column A of the current row. (This is actually `0`, but I don't like lifting my finger from the shift key.)
* `:$` : Goto the last column of the current row with meaningful data in it.
* `:^` : Goto row 1 of the current column.
* `:#` : Goto the last row of the current column with meaningful data in it.
* `vv` : replace the current cell with its evaluated value

### Edit Mode
Edit mode is entered when you start entering a label or formula.
* Left / Right : change cursor location
* Up / Down : End edit mode and navigate the pressed direction
* Page Up / Page Down : End edit mode and navigate Left / Right, respectively
* Insert : Toggle between insert / overwrite mode
* Delete : Delete character at cursor
* BackSpace : Delete character before cursor
* Home : Move cursor to beginning of input
* End : Move cursor to end of input

The sheet automatically recalculates after you finish entering a label or formula, and when you paste a cell. If a cell references a cell that hasn't been computed yet, then that cell will be computed, unless we are already in the process of computing that cell (circular reference).


## Entering Data

The easiest mode is label mode: it is just free-form text. It should probably work for non-English text if the console is in UTF-8 mode, maybe. It hasn't been tested and may not be compiled right.

Then, there is formula mode. Formula mode accepts formulas. It uses A1 cell references, despite immediately converting them to R1C1 under the covers, and thus the $A$1 fixed-reference format. Anchoring to a row or column only matters when you are copy/pasting cells, or creating LET names. Formulas can refer to cells, have constants in them (though scientific notation is not supported), perform addition `+`/subtraction `-`/multiplication `*`/division `/`, select ranges `:`, or call functions `@FUN`. Functions start with `@` like in early spreadsheet applications; their arguments are separated by semicolons `;`. You can do comparisons with `=`, `>=`, `<=`, `>`, `>`, or `<>`: the result is 1 for true and 0 for false. Use `&` to concatenate the string representations of two cells. The only half-attempt at internationalization that is supported is that `12,5` and `12.5` are treated the same. When you type a comma on numeric input, all of the numeric outputs will change to displaying a comma as the decimal separator.

One can also add strings to functions, as `"string"`. To put a double-quote in a string, do something like `"str""ing"` for `str"ing`.

Next, there are "names". Anything that begins with an underscore (`_`) is a name. Names are to make formulas easier to understand, but do note that they are defined within cells in the spreadsheet and need to be defined before use. One may want a cell that says `_GrossSales-_GrossCosts` to get their net profit. To do this is, put the formula `@LET("_GrossSales";$F$15)` in `A1`; and the formula `@LET("_GrossCosts";$G$19)` in `A2`. Now the cell `L15` could contain `_GrossSales-_GrossCosts`, assuming the default evaluation order. The `LET` function will not complain if the name is bad, but the parser only recognizes letters, numbers, underscores, and dollar signs (and it IS case sensitive, unlike function names or cell references). A mistake to consider, that the author ran into: `12` was in `A1`; `@EVAL("_Bob")` was in `A2`; and `@LET("_Bob", $A$1)` was in `A3`. While the author was editing `A2`, it evaluated to 12; immediately afterwards, however, it evaluated to Nil. The editor hid the error that the author was trying to use "_Bob" before it was defined in the evaluation order.

Finally, reference tables in the SQLite file under analysis with `!tableName` as in `A0!sqlite_schema` or `A0:D0!sqlite_schema`. For example, I found a sample database online with some song information, with the length, in milliseconds, in column G. To get the total length of all songs: `@SUM(G1:G3500!songs)`. However, I have been having some difficulty with some things: I found a sample database with census information about live births titled babynames. I've been having trouble translating this query into spreadsheet: `SELECT SUM(count) FROM babynames WHERE name='Thomas' and sex='M';`.

Example:  
`A$1+@SUM(C2:D3)+4/7`



## Standard Library

The following functions are all that is implemented. It is a curated list from the first version of VisiCalc, plus two functions that seem important. You can see the implementation in `Forwards/Tests/StdLib.txt`. If you load a library that redefines a function, it will successfully redefine that function. This can be used to improve the standard library (even though it is compiled into the program). For instance, the `limit_scale.txt` script overwrites SETSCALE to limit setting the scale to 10000 digits (from within the spreadsheet).

* MIN (%) - for functions marked (%), input is a variable number of arguments that can also be cell ranges. Empty cells and cells with labels are ignored. NaN is treated as an error value, not a missing value.
* MAX (%) - also, MIN and MAX return 'Empty' when given an empty set
* SUM (%)
* COUNT (%)
* AVERAGE (%) - literally SUM / COUNT
* NAN - returns the special Not-a-Number value
* ABS - absolute value
* INT - truncate to integer (return value has scale 0)
* ROUND - round to integer (ties away from zero) (return value has scale 0)
* GETSCALE - gets the scale setting
* SETSCALE - sets the scale setting
* GETROUND - gets the rounding setting
* SETROUND - sets the rounding setting
* EVAL - evaluate a label as a cell
* LET - define a name


## New Standard Library

This adds some transcendental functions that I didn't want to include at first. EXP, SQRT, and LOG draw heavy inspiration from GNU bc's implementation of the same. POW makes arguments ungodly precise to produce good results. This library probably only works well with Number System `-0`.

* EXP - The base of the natural logarithms raised to the argument power
* LOG - The natural logarithm
* SQRT - Square root
* RAISE - The first argument raised to an integer power second argument
* POW - The first argument raised to an arbitrary power second argument
* PMT - (interest rate per repayment period; repayment periods; present value)
* GETLIBROUND - Get the internal rounding mode of EXP, LOG, SQRT, and POW.
* SETLIBROUND - Set the internal rounding mode of those functions.


# Backwards

All of the scripting utilizes this language called Backwards. All functions that are exposed to the spreadsheet must have a name that is ALL CAPS and contain no numbers or underscores. This function must take one argument, and the argument is the array of arguments. Note, that it does lazy evaluation, so only the arguments to a function call that the function explicitly asks for will be evaluated. Also, also note: there is no way for a function to change the value of a cell except by returning the value that the current cell ought to have; there is no way for a function to look up the value of an arbitrary cell, all cells that it is to consider MUST be passed to it.

Example:  
```
set IF to function (x) is
   if EvalCell(x[0]) then
      return x[1]
   end
   return x[2]
end
```

Note that the call `@IF(3;5;1/0)` is completely valid. The 'else' clause will not be evaluated because it is never used, so the division-by-zero exception is never thrown.

## The Language
This is the language as implemented. Some of the GoogleTests have good examples, others, not so much.

### Data Types:
* Float - this is the arbitrary-precision decimal fixed-point number, and I was too lazy to change the name
* String
* Array (ordinally indexed Dictionary?)
* Dictionary
* Function Pointer
* Spreadsheet Expression Reference (CellRef)
* Spreadsheet Range

### Operations
* \+  float addition; string catenation; for collections, the operation is performed over the contents of the collection
* \-  float subtraction; for collections, the operation is performed over the contents of the collection
* \-  float unary negation; for collections, the operation is performed over the contents of the collection
* \*  float multiplication; for collections, the operation is performed over the contents of the collection
* /   float division; for collections, the operation is performed over the contents of the collection
* !   logical not
* \>  greater than, only defined for strings and floats
* \>= greater than or equal to, only defined for strings and floats
* <   less than, only defined for strings and floats
* <=  less than or equal to, only defined for strings and floats
* =   equality, defined for all types
* <>  inequality, defined for all types
* ?:  ternary operator
* &   logical and, short-circuit
* |   logical or, short-circuit
* []  collection access
* .   syntactic sugar for collection access; x.y is equivalent to x["y"]; in addition . and , are interchangeable, so x.y is the same as x,y
* {}  collection creation: `{}` is an empty array; `{ x; y; z }` creates an array; `{ x : y ; z : w ; a : b }` creates a dictionary

### Operator Precedence
* ()  -- function call
* {}
* . []
* ^
* ! -  -- unary negation
* \* /
* \+ \-
* = <> > >= < <=
* | &
* ?:

### Statements
I'm going to mostly use BNF. Hopefully, I'm not doing anything fishy. Note that [] is zero-or-one and {} is zero-to-many.
#### Expression
`"call" <expression>`  
To simplify the language, we have specific keywords to find the beginning of a statement. The "first set" of a statement is intentionally small, so that parsing and error recovery is easier. It makes the language a little verbose, though. Sometimes you want to just call a function (for instance, to output a message), and this is how. It will also allow you to add three to the function's result.
#### Assignment
`"set" <identifier> { "[" <expression> "]" } "to" <expression>`  
`"set" <identifier> { "." <identifier> } "to" <expression>`  
Assigning to an undefined variable creates it. The parser will catch if you try to create a variable and access it like an array/dictionary in the same statement. However, the parser probably won't catch `set x to x` until runtime.  
#### If
`"if" <expression> "then" <statements> { "elseif" <statements> } [ "else" <statements> ] "end"`  
#### While
`"while" <expression> [ "call" <identifier> ] "do" <statements> "end"`  
The "call" portion gives the loop a name. See break and continue.
#### For
`"for" <identifier> "from" <expression> ( "to" | "downto" ) <expression> [ "step" <expression> ] [ "call" <identifier> ] "do" <statements> "end"`  
`"for" <identifier> "in" <expression> [ "call" <identifier> ] "do" <statements> "end"`  
The second form iterates over an array or dictionary. When a dictionary is iterated over, the loop control variable is successively set to a two element array of { key, value }. If the variable does not exist, it will be created, and it will remain alive after the loop.
#### Return
`"return" <expression>`
#### Select
`"select" <expression> "from"`  
`    [ "also" ] "case" [ ( "above" | "below" ) ] <expression> "is"`  
`    [ "also" ] "case" "from" <expression> "to" <expression> "is"`  
`    [ "also" ] "case" "else" "is"`  
`"end"`  
Select has a lot of forms and does a lot of stuff. Cases are breaking, and "also" is used to have them fall-through. Case else must be the last case.
##### Break
`"break" [ <identifier> ]`  
Breaks out of the current while or for loop, or the named while or for loop. This isn't completely a statement, in that it requires a loop to be in context.
##### Continue
`"continue" [ <identifier> ]`  
##### Function Definition
`"function" [ <identifier> ] "(" [ <identifier> { ";" <identifier> } ] ")" "is" <statements> "end"`  
This is actually an expression, not a statement. It resolves to the function pointer of the defined function. As such, a function name is optional (but assists in debugging). Function declaration is static, all variables are captured by reference (no closures), and a function cannot access the variables of an enclosing function. All arguments to function calls are pass-by-value, semantically.  
New addition : the function name must now be unique with respect to: all functions that are being defined, global variables, and scoped variables. Functions can use their name to recursively call themselves.  
Previous way:  
`set fib to function (x) is if x > 1 then return fib(x - 1) * x else return 1 end end`  
New way:  
`set x to function fib (y) is if y > 1 then return fib(y - 1) * y else return 1 end end`  
Newer addition : closure / runtime parameterized functions  
Before the function name, add a list of expressions to capture in brackets. After the argument list, add a bracketed list of names for those values to have in the function. This list will be evaluated when the function pointer is used, and becomes part of the type of the function. These values parameterize the function.  
To create an Info function which decorates the original Info function:  
`set Info to function [Info] decorated_info (x) [y] is return y("Decorated: " + x) end`  
Parameters are also required when calling a function recursively:  
`set x to function [3] fib (y) [z] is if y > 1 then return fib[z](y - 1) * y else return 1 end end`

### Comments
`"(*" Comment "*)"`  
Old-style Pascal comments really round out the language as being valid even when newlines are replaced with spaces.

### Standard Library
* float Abs (float)  # absolute value
* float Ceil (float)  # ceiling
* value CellEval (string)  # parse and evaluate the given string as a cell expression, return its evaluated value
* float ContainsKey (dictionary, value)  # determine if value is a key in dictionary (the language lacks a means to ask for forgiveness)
* string DebugPrint (string)  # log a debugging string, returns its argument
* float EnterDebugger ()  # enters the integrated debugger (if present), returns zero
* string Error (string)  # log an error string, returns its argument
* value Eval (string)  # parse and evaluate the given string, return its evaluated value
* value EvalCell (CellRef)  # evaluate the CellRef, return its evaluated value
* array ExpandRange (CellRange)  # expand the CellRange: 2d cell ranges return a column-major array of cell ranges; 1d cell ranges return an array of CellRefs
* Fatal (string)  # log a fatal message, this function does not return, calling this function stops execution
* float Floor (float)  # floor
* float FromCharacter (string)  # return the ASCII code of the only character of the string (the string must have only one character)
* value GetIndex (array; float)  # retrieve index float from array
* array GetKeys (dictionary)  # return an array of keys into a dictionary
* float GetDefaultPrecision () # returns the current global scale variable
* float GetPrecision (float) # returns the scale of the passed in float
* float GetRoundMode () # returns a numeric representation of the current rounding mode
* value GetValue (dictionary; value)  # retrieve the value with key value from the dictionary, die if value is not present (no forgiveness)
* string Info (string)  # log an informational string, returns its argument
* dictionary Insert (dictionary; value; value)  # insert value 2 into dictionary with value 1 as its key and return the modified dictionary (remember, this DOES NOT modify the passed-in dictionary)
* float IsArray (value)  # run-time type identification
* float IsCellRange (value)  # run-time type identification : the result from evaluating a cell and it being a cell range
* float IsCellRef (value)  # run-time type identification : the raw arguments passed into a function from the runtime
* float IsDictionary (value)  # run-time type identification
* float IsFloat (value)  # run-time type identification
* float IsFunction (value)  # run-time type identification
* float IsInfinity (float)  # too big?
* float IsNaN (float)  # is this not a number?
* float IsNil (value)  # run-time type identification : the result from evaluating a cell and it having no contents
* float IsString (value)  # run-time type identification
* float Length (string)  # length
* float Max (float; float)  # if either is NaN, returns NaN; returns the first argument if comparing positive and negative zero
* float Min (float; float)  # if either is NaN, returns NaN; returns the first argument if comparing positive and negative zero
* float NaN ()  # returns the special not-a-number value
* array NewArray ()  # returns an empty array
* array NewArrayDefault (float, value)  # returns an array of size float with all indices initialized to value
* dictionary NewDictionary ()  # returns an empty dictionary
* array PopBack (array)  # return a copy of the passed-in array with the last element removed
* array PopFront (array)  # return a copy of the passed-in array with the first element removed
* array PushBack (array; value)  # return a copy of the passed-in array with a size one greater and the last element the passed-in value
* array PushFront (array; value)  # return a copy of the passed-in array with a size one greater and the first element the passed-in value
* dictionary RemoveKey (dictionary; value)  # remove the key value or die
* float Round (float)  # ties to even
* array SetIndex (array; float; value)  # return a copy of array where index float is now value
* float SetDefaultPrecision (float) # sets the current global scale variable; returns its argument
* float SetPrecision (float, float) # sets the scale of the first argument to the second argument; returns the modified argument
* float SetRoundMode (float) # sets the current rounding mode by number; returns its argument
* float Size (array)  # size of an array
* float Size (dictionary)  # number of key,value pairs
* float Size (CellRange)  # number of columns or rows (if there is only one column)
* float Sqr (float)  # square
* float SubString (string; float; float)  # from character float 1 to character float 2 (java style)
* string ToCharacter (float)  # return a one character string of the given ASCII code (or die if it isn't ASCII)
* string ToString (float)  # return a string representation of a float: scientific notation, 9 significant figures
* float ValueOf (string)  # parse the string into a float value
* string Warn (string)  # log a warning string, returns its argument

### Expanded Cell Ranges
Originally, a cell range was an opaque type in which the only allowed operation was expanding it into an array. I decided to streamline that and make a cell range a collection type. Now, You can call `GetIndex` and `Size` with a range, use `[]` to index into it, and iterate over it with `for ... in ...`. This should be more efficient if you are passing in a really large range.

### Rounding Mode Decoder Ring
Directed rounding modes are a part of IEEE-754 for doing algorithm analysis. Basically, it's a simple idea: change the rounding mode and see how the result changes. Note that the rounding mode only applies to addition, subtraction, multiplication, and division. A note on terminology: rounding to nearest means that behave as though we did the math to get the next digit, and then if the digit is 6-9, we round away from zero, and if 1-4 we round toward zero. A 5 is a "tie", and the round-to-nearest modes all specify how ties are handled, with "to even" and "to odd" meaning to make the least-significant digit of the result even or odd, respectively. Also note that the algorithms do analysis such that the next digit is only considered a 5 if it was EXACTLY a 5. If the next few digits are 500000001, then it is treated as a 6.
* 0 - Round to nearest, ties to even
* 1 - Round to nearest, ties away from zero
* 2 - Round to positive infinity
* 3 - Round to negative infinity
* 4 - Round to zero
* 5 - Round to nearest, ties to odd
* 6 - Round to nearest, ties to zero
* 7 - Round away from zero
* 8 - Round 05 away from zero : double rounding mode

The final rounding mode probably needs some explanation: it is round to zero, unless the last digit of the result is a zero or five, then it is round away from zero. This mode allows computations made at a higher precision to be double rounded later to a lower precision (correctly). The least significant digit is treated as a sticky digit ought to be for eventual rounding.

NOTE: Some testing violently reminded me that the algorithms for EXP and LOG don't work in rounding modes 2 or 7 (and LOG doesn't work in 3 either). They expect a value to go to zero that can never go to zero: we divide two numbers expecting to eventually get zero but these rounding mode prohibits this. The termination conditions for LOG and EXP were reworked so that they can be used in all rounding modes. Also, the algorithm for SQRT (and thus LOG) did not work in rounding mode 8. For whatever reason, it is very likely to hit cases where it bounces between two solutions one unit in the last place apart. You would think that implies that the SQRT algorithm was broken for all rounding modes, but I haven't seen cases in the wild that are broken for other rounding modes. Anyway, this has been fixed (if the last two solutions differ by an ulp, then we increase working precision).  
In addition to this, the functions LOG, EXP, POW, and SQRT have been modified so that they internally use the new double rounding mode. This should make the output more consistent when you change the rounding mode. Is it the right thing to do? Probably not. However, it hides the numerical instability of those algorithms when one is analyzing some other algorithm. There's an irony that I changed the algorithms to work with any rounding mode, then changed them to always use a rounding mode that they always worked with (except SQRT).
