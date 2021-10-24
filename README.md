# little-lang
Compiler and tools for the Little programming language.

## Examples
### Hello World
```rs
fn main() : void {
  (print "Hello, World!\n")
}
```
### Add Two Numbers
```rs
fn add(int a, int b) : int {
  (return (+ a b))
}

fn main() : void {
  (defvar sum (add 5 10))
  (printi sum)
}
```
## Building, Testing & Compiling
### Building
To build the `Little` compiler, simply run:
```
make
```
> Note: This will generate a binary file in the `bin` directory called `little`.
### Testing
The test cases can be run with the `--test` flag:
```
bin/little --test
```
> Note: They do not they all possible cases, but most of them are covered.
### Compiling
To compile a file, use the generated binary from the build step:
```
bin/little test_file.lil
```
> Note: This will generate another binary in the `bin` directory called `a`.

## Features
- [x] Addition (+)
- [x] Subtraction (-)
- [x] Multiplication (*)
- [x] Division (/)
- [x] Modulus (%)
- [x] Conditions
- [x] Function Definitions
- [x] Function Calls
- [x] Comments
- [x] Integers
- [ ] Floats
- [ ] Structs
- [ ] Enums
- [x] Arrays
- [ ] Scopes
- [x] If Statements
- [x] For-Loops
- [x] While-Loops
- [x] Variable Assignments

## References

[chibicc](https://github.com/rui314/chibicc/)
