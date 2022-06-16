# ShnooTalk Compiler Test Suit

These are end to end tests which can be used to test an implementation of ShnooTalk. The `tests_runner` python application can be used 
to run the tests. The `tests_runner` application expects the output executable to be in `bin/debug/shtkc` or `bin/gcov/shtkc` (for coverage). 
See `COMPILER_EXEC_PATH` in `tests_runner/framework/config.py` for more info.

## Directories

### parser/

This is meant to test the AST generated from the parser and the pretty printer that prints the AST.

### compiler/

Test the output executable produced by the compiler and the generated ShnooTalk icode (not the entire IR just the generated function IR instructions only, see `-icode-all` option).
This is also meant to test the compiler's pretty printer (the part that prints function code).

If you are writing you own implementation of ShnooTalk, it is enough to make it pass the compiler tests alone the other tests are optional (probably you will have to write you own).

### ir/

This is meant to test the overall structure of the generated IR (i.e the parts that were not tested in compile/ tests). This is also meant to test the IR 
pretty printer.

## When to add a test?

+ When you discover a compiler bug
+ When you add a new grammar or syntax
+ If you make changes to existing syntax, you will most probably just have to fix failing tests
+ When you make changes to the IR or the IR builder
+ Avoid making changes to the pretty printer or modifying syntax of existing IR 
  instructions, but if you do, you may have to change a lot of test cases, re-generate the test cases,
  and do careful review using `git diff`

## Where to add the test?

+ If you made changes to the parser, add test to the parser/ directory
+ If you have made changes to the icode (IR instructions) or added a new IR instruction, add test to the compile/ directory
+ If you have fixed a compiler bug, add a test to compiler/ directory
+ If you have made changes to the IR but not icode (IR instructions), add test to the ir/ directory

## How to run the tests?

+ Run `make test` or `make coverage`

## How to generate the test cases?

+ Run `make gen`
+ Carefully review the changes using `git diff`