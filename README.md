# GLE
General Language Environment version 2 implementation

# General Overview
General Language Environment aims to provide a generic runtime for programming languages.
Its memory management system is based on the underrated Kreczmar's and Cioni's system for programmed deallocation of objects.
This means that there is no dangling reference problem at all! If you don't know, what is that, read [the section below](#dangling-reference-problem).

Genericity is going to be implemented by:
- extensible instruction set
- usage of a memory management module of your preference; [read more about available ones](#memory-modules)


# Core Instruction Set
Documentation can be found here: [https://github.com/ankiedos/gle/blob/gle-2/docs/cis.md](https://github.com/ankiedos/gle/blob/gle-2/docs/cis.md)

# format:
- always LSB -> MSB
- memory module to use (imp, obj, decl, blkchn, reldb)
- instruction plugins to load
- performance config:
  - compactification frequency
  - compactification effectivity

# Roadmap
- [ ] add run-time disassembler in debug mode
- [ ] add emit() that throws error signals
- [ ] encode addressing modes in a single word
- [ ] add 0xCC and 0xFF modifiers to notify about modifcation of code or registers, respectively [here's the full idea](#instruction-prefixes)


# Dangling Reference Problem
Dangling reference problem happens, when an object has been deallocated, but there exists a pointer to it, which hasn't been nullified. In such a situation,
when the programmer wants to access the memory frame the pointer points to...
one of the two things can happen:
- (a) the pointer may point to a memory location now locked by another program
- (b) the pointer may point to a memory location now used by another resource
- (c) the pointer may point to an already released memory location

Kreczmar-Cioni's system makes use of indirect pointers, to a direct pointer, and both of them store counters, whose values are different, when the direct pointer changes the frame it points to, by incrementing the counter of the direct pointer.

With this algorithm, the nullification of each indirect pointer happens in the constant time!

Let's demonstrate it using an example.

Suppose we have a pointer variable `x` that points to the object `o` of type `T`. `x := new T()`. Now, let's add another pointer `y := x`.
In the next step, deallocate the object using the `x` pointer: `kill x, x := null`.
Some lines after, suppose we use the pointer `y`: `print(to_string(y))`.

### **BOOM!**

The object that `y` pointer points to, doesn't exist! `UndefinedReferenceException` was thrown!

With Kreczmar-Cioni's system, `kill x` is equivalent to `kill o, x := y := null`.

# Memory Modules
There will be available a bunch of different memory management modules. Currently planned are:
- [x] object memory
- [ ] imperative memory
- [ ] functional memory
- [ ] relational DB memory
- [ ] blockchain memory
- [ ] AI memory

# Instruction Prefixes
When there's `0xCC` before an opcode, `vm.current_resource` is set to `vm.c`. Then, when the instruction is being executed (e.g. `iadd`):
`vm.current_resource[dst] = lhs + rhs;` changes the byte at index `dst` of `vm.c`.


On the other hand, when there's `0xFF` before an opcode, `vm.current_resource` is set to `vm.regs`. Then, when the instruction is being executed (e.g. `isub`):

`vm.current_resource[dst] = lhs - rhs;` changes the register numbered `dst`.