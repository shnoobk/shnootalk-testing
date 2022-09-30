# ShnooTalk language guide


🦄 Emoji in the document indicates notes about missing features and something that might change in the future.

## Comments

Comments in ShnooTalk start with `#` character.

```
# This is a comment
const a := 1  # This is another comment
```

## Literals

### Integer literals

Integer literals can be written in hex, binary or decimal.

```
1234        # decimal
0xABCD      # hex
0xabcd      # also hex
0b101101    # binary
```

### Float literals

Floating point literals are made up of numbers with `.` in the middle.

```
1.234
0.234
.234  # invalid float literal
```

### Character literals

Character literals store a single character and are inclosed within single quotes (`'`). 

```
'c'
'a'
'\n'
```

### String literals

String literals are enclosed within double quotes (`"`). Internally string literals are represented as null terminated char arrays.

```
"Hello world"
"This is another string\n"
```

### Multiline string literals

Long string literals can be split up into multiple parts to improved readability. The parts or concatenated into one single string without any delimiter.

Below three statements are all equivalent

```
const a := "Hello world"

const b := "Hello" " world"

const c :=
    "Hello"
    " World"
```

### The `def` keyword

If a literal is used in multiple places, it can be aliased or given a name to improve readability. Multiline string literals cannot be used with `def`.

```
def PI 3.14
def HELLO_WORLD "Hello world"
def ZERO 0

println(PI)             # equivalent to println(3.14)
println(HELLO_WORLD)    # equivalent to println("Hello world")
println(ZERO)           # equivalent to println(ZERO)
```

### Builtin literals

The following literal are already pre-defined in ShnooTalk

| Name     | Value    | Type |
|----------|----------|------|
| false    | 0        | integer
| true     | 1        | integer
| NULL     | 0        | integer
| NAN      | nan      | floating point
| INFINITY | infinity | floating point

## Program entry point

All executable programs must have a function called `main`. When a program starts, this is the first function that will be called, and it must return a `int` primitive type.

```
fn main() -> int
{
    println("Hello world")
    return 0
}
```

## Types

### Primitive types

ShnooTalk has the following primitive types

| Name               | Description |
|--------------------|-------------|
| `byte`             | Signed 8-bit integer.
| `ubyte` or `char`  | Unsigned 8-bit integer.
| `short`            | Signed 16-bit integer.
| `ushort`           | Unsigned 16-bit integer.
| `int`              | Signed 32-bit integer. This is the default integer type.
| `uint`             | Unsigned 32-bit integer.
| `long`             | Signed 64-bit integer.
| `ulong`            | Unsigned 64-bit integer. This is used to represent sizes in ShnooTalk 🦄.
| `float`            | 32-bit floating point number.
| `double`           | 64-but floating point number. This is the default for floating point number.
| `bool`             | Alias for `byte`


🦄 This will change to a new primitive type called `usize` in the future.

### Arrays

Arrays store multiple values of the same type on the stack. They can be indexed and declared using the `[n]` syntax. Arrays are indexed starting from zero.

```
fn main() -> int
{
    const a: int[3] = [10, 20, 30]

    println(a[1])   # prints 20

    return 0
}
```

🦄 There is no runtime or compile time bounds checking performed when an array is indexed as of now, so arrays must be used with caution. This will change in the future.

🦄 Users are encouraged to use `List` from the standard library instead of arrays, which has bounds checking, but this allocates values on the heap.


### Pointers

Pointer types store memory addresses instead of a value, hence pointers can *reference* or *point to* another variable and any modification made to the pointer will also reflect on the variable. The `<-` operator is used to make or change what variable the pointer is pointing.

Pointers are declared using the `*` syntax.

```
fn main() -> int
{
    var a: int = 2
    var ptr: int* <- a

    ptr += 2        # This will modify a

    println(a)      # will print 4

    var b: int = 10
    
    ptr <- b        # ptr now points to b instead of a

    println(ptr)    # prints 10 which is the value of b

    return 0
}
```

 🦄 Pointers and arrays are very unsafe in ShnooTalk and must be used minimally and only if required. Users are encouraged to use types from the standard library instead. Operations on pointers maybe only permitted inside `unsafe` blocks in the future.

### Array pointers

Array pointers are the same as pointer but for arrays and are indexable using the `[n]` syntax.

Array pointers are declared using the `[]` syntax without any literal between the brackets.

```
fn main() -> int
{
    var a: int[3] = [1, 2, 3]
    var b: int[] <- a

    b[1] = 200  # this will modify a

    println(a[0], a[1], a[2])   # prints 1 200 3

    return 0
}
```

### Structs

Struct are user defined types. It lets the user define a new type composed of multiple different or same primitive types.

```
struct Player
{
    var id: int
    var x: double, y: double
}

fn main() -> int
{
    var a: Player

    a.id = 1
    a.x = 2.3
    a.y = 4.5

    println(a.id, a.x, a.y)

    return 0
}
```

🦄 There will be a better syntax to initialize all struct fields in the future.

🦄 The compiler does not pad struct for alignment yet. This feature might be added in the future. Currently struct types are packed without any padding.

### Enums

Enums lets the user define a integer type that can only have limited range of continues values. Enums are usually used to type categorical types like colors, error types etc.

```
enum TempRange {LOW, MEDIUM, HIGH}

fn main() -> int
{
    var a: TempRange = LOW

    println(a)  # prints 0

    a = HIGH

    println(a)  # prints 2

    a = 3   # compile error, this results in type error

    return 0
}
```

## Variables

### Global variables

Global variable are declared outside all functions, they are accessible to all the functions declared in the same file.

Global variables can only be initialized inside a function and cannot be `const`.

```
var a: int

fn foo() 
{
    a += 1
}

fn bar()
{
    println(a)
}

fn main() -> int
{
    a = 10  # initialize global variable
    foo()
    bar()   # prints 11
    return 0
}
```

🦄 There might be syntax for declaring an init function for a file to initialize global variables that will be automatically called before the program starts in the future.

### Local variables

Local variables are declared inside a block enclosed with `{` and `}` and are only accessible inside the same block they are declared or withing a nested block. They cannot be accessed in the parent block.

```
fn main() -> int
{
    var a: int = 1

    if a == 1 {
        println(a)  # a is accessible inside nested blocks

        var b: int = 2
        println(b)  # b is accessible inside the same block it is declared in
    }

    println(b)  # compile error, b is not accessible in the parent block

    return 0
}
```

Local variables can be initialized either using `var` or the `const` keyword. Variables initialized using the `const` keyword cannot be modified later in the program and providing an initial value is compulsory.

```
fn main() -> int
{
    var a: int
    a = 1   # can be initialized later
    const b: int = 1

    a += 1
    println(a)  # prints 2

    b += 1  # compile error, b cannot be modified

    return 0
}
```

The type can be ignored by using `:=` operator instead of `=`. Using `const` and `:=` is preferred over `var` and `=` in ShnooTalk. 

```
fn main() -> int
{
    const a := 1      # default type int is used

    println(a)

    return 0
}
```

For pointers, `:=` or `=` syntax cannot be used and `<-` operator must be used instead. Type must always be explicitly specified.

```
fn main() -> int
{
    var a := 1
    var ptr: int* <- a

    println(ptr)    # prints 1

    return 0
} 
```

## Assignments

The `=` operator can be used to change the value of a variable.

```
fn main() -> int
{
    var a := 1
    println(a)
    a = 2   # change to 2
    println(a)  # prints 2
    return 0
}
```

For pointers, `=` operator modifies the variable the pointer is pointing or referencing. And `<-` operator can be use to change what variable the pointer is pointing [as seen here](#pointers).

There are also other assignment operators other than `=`. These are `*=`, `/=`, `%=`, `+=`, `-=`, `<<=`, `>>=`, `&=`, `^=`, `|=`.

```
fn main() -> int
{
    var a := 1
    println(a)
    a += 1  # equivalent to a = a + 1
    println(a)  # prints 0
    return 0
}
```

## Destructuring

The destructuring syntax allows unpacking arrays and struct into variables in assignment or declaration/initialization.

### Ordered destructuring

Unpack values from struct or array in order and assign it to a variables.

```
fn main() -> int
{
    var [a, b] := [1, 2]  # declare a and b and unpack this array into a and b

    println(a)  # prints 1
    println(b)  # prints 2

    .[a, b] = [3, 4]   
    # The preceding dot is purely syntactic
    #   and is there to make grammar parsable
    #   the dot can be ignored in some cases but 
    #   it is recommended to have it anyways

    println(a)  # prints 3
    println(b)  # prints 4

    return 0
}
```

It is also possible to unpack a struct into variables.

```
struct Point
{
    var x: int, y: int
}

fn main() -> int
{
    var p: Point
    p.x = 1     
    p.y = 2

    const [a, b] := p

    println(a)  # prints 1
    println(b)  # prints 2

    return 0
}
```

It is also possible to initialize struct fields using destructuring.

```
struct Point
{
    var x: int, y: int
}

fn main() -> int
{
    var p: Point
    .[p.x, p.y] := [1, 2]   # unpack array elements into struct fields of p

    println(p.x, p.y)  # prints 1 2

    return 0
}
```

The unpacking and assignment always happens from left to right, so this CANNOT be used to swap value of variables, so below will not work.

```
fn main() -> int
{
    var [a, b] := [1, 2]

    .[a, b] = [b, a]

    print(a, b)     # print 2 2 and NOT 2 1     

    return 0
}
```

### Named destructuring

When right hand side of destructuring assignment or initialization is a struct and left hand side contains only identifiers whose names are present as a field in the struct, ShnooTalk will perform a named destructuring. It will unpack the field into the variable with same name as the field.

ShnooTalk will try to perform named destructuring first and fallbacks on ordered destructuring if it is not possible.

```
struct Point
{
    var x: int, y: int
}

fn main() -> int
{
    var p: Point
    p.x = 1     
    p.y = 2

    const [y, x] := p   # performs named structuring

    println(x)  # prints 1  (would have been 2 if ordered destructuring)
    println(y)  # prints 2  (would have been 1 if ordered destructuring)

    return 0
}
```

Unlike ordered destructuring, the number of elements on left and right don't have to match.

```
struct Point
{
    var x: int, y: int
}

fn main() -> int
{
    var p: Point
    p.x = 1     
    p.y = 2

    const [y] := p   # performs named structuring

    println(y)  # prints 2

    return 0
}
```

## Expressions

You can use expressions in ShnooTalk to do computations and calculations.

```
fn main() -> int
{
    const a := 1 + 2
    println(a)  # prints 3

    return 0
}
```

### Binary operators

ShnooTalk has the following binary operators

| Symbol | Name |
|--------|------|
| *      | Multiply
| /      | Divide
| %      | Remainder or modulo
| +      | Add
| -      | Subtract
| <<     | Left shift
| >>     | Right shift
| &      | Bitwise and
| ^      | Bitwise xor
| \|      | Bitwise or

### Unary operators

ShnooTalk has the following unary operators

| Symbol | Name |
|--------|------|
| -      | Minus
| ~      | Bitwise not

### Conditional expressions

You can use conditional expression in ShnooTalk to evaluate and check conditions. Conditional expression result to an integer that is either 0 or 1. The result of a conditional expression can be assigned to any integer type but it is recommended to use the `bool` type.

```
fn main() -> int
{
    const a := 4

    println(a < 3)              # prints 0
    println(a >= 4)             # prints 1
    println(a > 1 and a < 5)    # prints 1

    return 0
}
```

### Relational operators

ShnooTalk has the following relational operators

| Symbol | Name |
|--------|------|
| <      | Less than
| <=     | Less than or equal to
| >      | Greater than
| >=     | Greater than or equal to
| ==     | Equal
| !=     | Not equal

### Logical operators

ShnooTalk has the following logical operators

| Symbol       | Name |
|--------------|------|
| `not` or `!` | Not
| `and`        | And
| `or`         | Or 

### Type casting

ShnooTalk does not allow binary operator or relational operators between two different primitive types. 

```
fn main() -> int
{
    const a := 2
    const b := 2.3

    println(a+b)    # compile error

    return 0
}
```

So you will have to explicitly tell ShnooTalk to cast types using the <code>`</code> type cast operator,

```
fn main() -> int
{
    const a := 2
    const b := 2.3

    println(double`a+b)     # cast a from int to double, prints 4.3

    return 0
}
```

### Pointer type casting

You can use the <code>*\`</code> and <code>[]`</code> operator to bitcast pointers from one type to another. This operator is very unsafe as it overrides ShnooTalk's type checking. This operator is useful to interface with C functions.

```
fn main() -> int
{
    var a := 65

    const ptrA: int* <- a
    const ptrB: char* <- char*`ptrA     # bitcast int* to char*

    println(ptrB)   # prints A which is 65 in ascii

    return 0
}
```

You can use <code>[]`</code> to bitcast array pointers.

```
extfn malloc(bytes: ulong) -> ubyte[]

fn main() -> int
{
    const a: int[] <- int[]`malloc(100)

    return 0
}
```

You can also cast integers into pointer

```
fn main() -> int
{
    const a: int* <- int*`NULL

    return 0
}
```

## Control statements

Control flow statements can change the flow of the program. Control flow statements in ShnooTalk are very similar to C but no parenthesis is required for the conditional statement.

### `if` statement

If statements have a compulsory starting `if` branch, then subsequent optional multiple `else if` branch and then final optional `else` branch.
```
fn main() -> int
{
    const a := 10

    if a > 5
    {
        println("A is greater than 5")
    } 
    else if a > 3
    {
        println("A is greater than 3")
    }
    else 
    {
        println("A is less than or equal to 2")
    }

    return 0
}
```

If there is only one statement inside the block, `{` and `}` can be ignored. This applies for all control statements.

```
fn main() -> int
{
    const a := 4

    if a > 2 
        println("A is greater than 2")
    else 
        println("A is less than or equal to 2")

    return 0
}
```

The entire statement could also be written in one line

```
fn main() -> int
{
    const a := 4

    if a > 2 println("A is greater than 2") else println("A is less than or equal to 2")

    return 0
}
```

If statements and other control statements can also accept an expression that results in any integer value instead of just 0 or 1, and anything not equal to zero will be treated as `true`

```
fn main() -> int
{
    if 2 println("Hello")   # prints hello

    return 0
}
```

### Infinite loop

Executes a set of statements continuously forever until a `break` is encountered.

```
fn main() -> int
{
    loop 
    {
        println("Hello world")  # prints Hello world forever
    }

    return 0
}
```

### `while` loop

While loop is used to execute a set of statements repeatedly while a condition remains true.

```
fn main() -> int
{
    var a := 1

    while a <= 4
    {
        println(a)      # prints 1 2 3 4
        a += 1
    }

    return 0
}
```

### `do while` loop

A do while loop is very similar to the while loop, except the condition is checked AFTER executing the statements unlike the while loop where the condition is checked BEFORE executing the statements. Hence the statements are executed at least one in a do while loop.

```
fn main() -> int
{
    var a := 4

    do
    {
        a += 1      
        println(a)  # prints 5, while loop would not print anything
    } while a < 4

    return 0
}
```

### `for` loop

A for loop is very similar to the while loop except it also lets you declare or assign a variable and provide an update statement.

```
fn main() -> int
{
    for var i := 1; i <= 10; i += 1
    {
        println(i)
    }

    return 0
}
```

This is equivalent to 

```
fn main() -> int
{
    var i := 1

    while i <= 10
    {
        println(i)
        i += 1
    }

    return 0
}
```

### `break` and `continue`

The `break` keyword is used to exit or abort a loop prematurely.

```
fn main() -> int
{
    for var i := 1; i <= 10; i += 1
    {
        if i == 5 break     
        println(i)      # prints upto 4 and not 10
    }

    return 0
}
```

The `continue` keyword can be used to skip a particular loop

```
fn main() -> int
{
    for var i := 1; i <= 10; i += 1
    {
        if i == 5 or i == 7 continue     
        println(i)      # prints upto 10 but skips 5 and 7
    }

    return 0
}
```

## Functions

### Basic functions

Functions in ShnooTalk can be declared with the `fn` keyword

```
fn sayHello()
{
    println("Hello")
}

fn main() -> int
{
    sayHello()
    return 0
}
```

The `return` statement can be used to return values from the function. The `->` token is used to mark the return type. Functions can also accept parameters.

```
fn add(a: int, b: int) -> int
{
    return a + b
}

fn main() -> int
{
    const a := add(1, 2)

    println(a)

    return 0
}
```

By default functions cannot modify the parameters passed to them, if you would like to do so, the parameter has to marked mutable with the `mut` token. If a parameter is marked `mut`, `const` variables cannot be passed as an argument.

```
fn modifyInPlace(mut a: int)
{
    a += 1
}

fn main() -> int
{
    var a := 1      # hs to be var, const won't work

    modifyInPlace(a)

    println(a)      # prints 2

    return 0
}
```

### Functions as constructors

Function can be used as constructors for `struct` type. The convention is the type's name is in *PascalCase* and the constructor function name is the same name as the type but in *camelCase*

```
struct Point
{
    var x: int, y: int
}

fn point(x: int, y: int) -> Point
{
    var p: Point
    p.x = x
    p.y = y
    return p
}

fn main() -> int
{
    const p := point(1, 2)

    return 0
}
```

### Methods

Functions whose first parameter is a `struct` type, will also become a method associated with the struct. Methods are almost just syntax sugar for calling a normal function in ShnooTalk. The first parameter is always named `self`.

```
struct Point
{
    var x: int, y: int
}

fn point(x: int, y: int) -> Point
{
    var p: Point
    p.x = x
    p.y = y
    return p
}

fn printPoint(self: Point)
{
    println(self.x, self.y)
}

fn add(self: Point, other: Point) -> Point
{
    return point(self.x + other.x, self.y + other.y)
}

fn main() -> int
{
    const p := point(1, 2)
    const q := point(3, 4)

    # Both statements are equivalent
    const a := p.add(q)
    const b := add(p, q)
    
    a.printPoint()  # prints 4 6
    b.printPoint()  # prints 4 6 also

    return 0
}
```

### `extfn` functions

If you want to use function from C libraries, you will have to declare them using the `extfn` keyword

```
extfn exit(status: int)

fn main() -> int
{
    exit(1)
    return 0
}
```

### `externC` functions

If you want to use ShnooTalk functions in C or export it from WASM module compiled from ShnooTalk or use it in other languages with C interoperability, the function has to be marked with the `externC` keyword. This will tell ShnooTalk not to name mangle the function. Using this keyword can also lead to linker errors if they are declared more than once in different modules with the same name unlike normal functions.

```
externC fn add(a: int, b: int) -> int
{
    return a + b
}
```

## Print statements

You can use the `print` and `println` keywords to print to console or terminal. `println` will add newline (`\n`) character at the end.

```
fn main() -> int
{
    print("Hello world\n")
    println("Hello world")  # no need for \n
    
    return 0
}
```

You can pass multiple values to the print statement, each value will be printed separated by a space character.

```
fn main() -> int
{
    # prints 1 2 Hello world
    println(1, 2, "Hello", "world")
    
    return 0
}
```

If you don't want the space between each value, you can use `;` instead of `,`

```
fn main() -> int
{
    # prints 12 Helloworld
    println(1; 2, "Hello"; "world")
    
    return 0
}
```

## Builtin functions

### `sizeof` builtin

This is a special function that accepts a type as a parameter and prints the total size that type occupies in memory in bytes. This function is run at compile time.

```
struct Point
{
    var x: int, y: int
}

fn main() -> int
{
    println(sizeof(int))        # prints 4
    println(sizeof(Point))      # prints 8

    return 0
}
```

### `addr` builtin

The `addr` function return the address that a pointer is holding. The function is very useful in checking if a pointer is null.

```
fn main() -> int
{
    var a := 1
    var ptr: int* <- a

    println(addr(ptr))  # print address of a

    if addr(ptr)
        println("ptr is not null")

    return 0
}
```

## Standard library

The guide has covered most of ShnooTalk's low level core syntax, but the language by itself does not have lot of features that a modern programing language would provide such as lists, dictionaries or maps, reading and writing files, generating random numbers etc. 

So ShnooTalk comes with a standard library written in low level ShnooTalk syntax using C functions from [libc](https://en.wikipedia.org/wiki/C_standard_library).

### String

Without the standard library strings are represented as null terminated char arrays.

```
fn main() -> int
{
    const message := "Hello world"
    println(message)

    return 0
}
```

Char arrays are not resizable and you cannot perform any string operations on them. Char arrays also don't have any bounds checking because they are [arrays](#arrays). So the standard library provides a String type.

```
fn main() -> int
{
    const message := "Hello world".string()
    println(message)

    return 0    
}
```

If you have used C++ you can thing of the two types as `std::string` and C strings or if you have used rust, `String::from` and string slice.

Here are some operations you can perform on the String type

```
from "stdlib/String.shtk" use string, parseInt

fn main() -> int
{
    var message := "Hello".string()

    message += " World"     # append string
    println(message)
    
    println(message.length())

    println(message[6])     # indexing

    println(message[[4, 8]])    # substring

    const number := "1234".parseInt().expect()  # parse to int
    println(number)

    println("ello" in message)  # check if substring exits
 
    return 0
}

```

Here are list of operations you can do on the String type

- `fn length(self: String) -> ulong`
- `fn isAlphaNumeric(self: String) -> bool`
- `fn isSpace(self: String) -> bool`
- `fn isUpper(self: String) -> bool`
- `fn isLower(self: String) -> bool`
- `fn isInteger(self: String) -> bool`
- `fn isNumber(self: String) -> bool`
- `fn lower(self: String) -> String`
- `fn upper(self: String) -> String`
- `fn split(self: String, delimeter: char) -> List[String]`
- `fn startsWith(self: String, other: String) -> bool`
- `fn endsWith(self: String, other: String) -> bool`
- `fn find(self: String, other: String) -> Optional[ulong]`
- `fn strip(self: String) -> String`
- `fn parseInt(self: String) -> Optional[int]`
- `fn parseLong(self: String) -> Optional[long]`
- `fn parseFloat(self: String) -> Optional[float]`
- `fn parseDouble(self: String) -> Optional[double]`
- Operators `+`, `==`, `!=`, `>`, `<`, `>=`, `<=`, `in`, `[]`, 
- `[[start, end]]` for substring

### List

### List utilities

### Dict

### Error handling

#### Result

#### Optional

#### Question mark operator

### Time

### Random

### Panic

### Returning multiple values

### Math

### Files

### Running subprocess

### OS
- env variable
- cli args


## TODO

- Naming conventions
- `make` builtin
- Module system
  - `::` expression
  - `from` and `use`
  - `SHNOOTALK_PATH` and default `/usr/local/lib`
- Mutability and pointer rules
  - Assignment
  - Function parameters
  - Local variables
  - Return statement
- Operator hooks
    - `__multiply` hook
    - `__divide` hook
    - `__mod` hook
    - `__add` hook
    - `__subtract` hook
    - `__rightShift` hook
    - `__leftShift` hook
    - `__bitwiseAnd` hook
    - `__bitwiseXor` hook
    - `__bitwiseOr` hook
    - `__lessThan` hook
    - `__lessThanOrEqual` hook
    - `__greaterThan` hook
    - `__isEqual` hook
    - `__isNotEqual` hook
    - `__in` hook
    - `__isNonZero` hook
    - `__subscript` hook
- Print hooks
    - `__toCharArray__` and `__toString__` hook
- Loop hooks
    - `for x in y` loop
    - Destructured `for x in y` loop
- Question mark operator hooks
    - `__questionMarkContinue__` hook
    - `__questionMarkUnwrap__` hook
    - `__questionMarkError__` hook
    - `__questionMarkConstruct__` hook
- Resource management hooks
  - `__beforeCopy__` hook
  - `__deconstructor__` hook
- Hooks for string and char types
    <!-- TODO remove this feature? -->
    - `__coerceCharArray__` hook     
- Generics
  - Type hint
  - Generic function call
  - Same name existence
- Compiler internals
    - Parser
    - Intermediate representation
    - LLVM