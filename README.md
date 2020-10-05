# Early program initialization

`main()` is not the first function your program runs. The ELF file includes additional mechanisms to perform an early initialization that can be run before `main()` gets control: using the `DT_INIT` and `DT_INITARRAY` keys.

The gcc compiler supports putting its own functions into the `DT_INITARRAY` keys through the *constructor* attribute.

Compiling the program is possible using the command line:

```$ gcc init.c -o init -ggdb -O0```

The program execution is presented in the following listing:

```$ gcc init.c -o init -ggdb -O0
$ gdb init
Reading symbols from init...
(gdb) b main
Breakpoint 1 at 0x1160: file init.c, line 8.
(gdb) r
Starting program: /home/s3jk1/GitHub/Runtime-on-GNU-Linux-systems/init 
Calling before main()!

Breakpoint 1, main () at init.c:8
8       int main() {
(gdb)```
