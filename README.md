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

## Features
- [x] Addition (+)
- [x] Subtraction (-)
- [x] Multiplication (*)
- [x] Division (/)
- [x] Mod (%)
- [x] Conditions
- [x] Function Definitions
- [x] Function Calls
- [x] Comments
- [x] Integers
- [ ] Floats
- [ ] Structs
- [ ] Enums
- [ ] Arrays
- [x] If Statements
- [x] For-Loops
- [x] While-Loops
- [x] Variable Assignments

## References

[chibicc](https://github.com/rui314/chibicc/)
