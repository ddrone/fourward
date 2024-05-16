# ＦＯＵＲＷＡＲＤ

Originally I've intended to write a simple RPN calculator in C, but got carried away and ended up implementing a small Forth-like programming language. It's nowhere near as capable or as elegant as Forth implementations, but at least I haven't had to suffer writing x86 assembly in order to get it out of the door. (I **had** to suffer writing C, which is, arguably, not that much of an improvement).

In this implementation, it's possible to write immediate words (the ones that get evaluated during compilation of other words), and these immediate words can use primitives to read characters from the source stream and write and modify bytecode, which enables primitive metaprogramming. The only control-flow primitives provided by the language are conditional and unconditional jumps; `conditional.f` implements `if-then-else` and `while` loops on top of that. Reading (nested) comments in parentheses is also implemented inside the language itself.

The only specific goal I had in mind while starting this project was to learn to debug programs in C, and I've quickly encountered one issue that I had to debug: after implementing simple conditionals, I've noticed that execution of programs in non-deterministic. It turned out that the function `step` that does one step of a virtual machine haven't had a `return` statement in the end of it. Since that return value controlled whether the execution should be continued, the program could have stopped at a random point in time. And, since even simple conditionals are implemented using metaprogramming, that quickly lead to bytecode being corrupted as well. Oops.

Turns out, you have to compile C code with `-Wall` in order for GCC to tell you that you forgot to add a `return` in the end of the function.
