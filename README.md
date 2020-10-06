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
(gdb)
```
In the sample `gdb(1)` session, you can clearly see that the function `init_func()` was called before control was obtained by the function `main()`. This technique can be used by malware to obstruct dynamic analysis with debuggers such as `gdb(1)`. Functions in the `DT_INIT` or` DT_INITARRAY` tags can use special methods to detect the existence of debuggers and virtual machines and change the logic of the program's operation in a way that makes analysis difficult.

The `DT_INITARRAY` key can also be used by the C++ compiler to initialize global objects. Using the example of a minimal C++ program, it is useful to follow the code localization process based on the addresses in `DT_INITARRAY`.

Compiling and running the program is limited to running the commands:

```$ g++ initarray.cpp -o initarray
$ ./initarray
pre-main
main
```

First, read the address of the board *initarray* and its size stored in the `DT_INITARRAYSZ` key:

```$readelf -d initarray | grep INIT_ARRAY
 0x0000000000000019 (INIT_ARRAY)         0x3db0 // In my case
 0x000000000000001b (INIT_ARRAYSZ)       16 (bytes)
```

The memory address should now be converted into a file address. Reading 16 bytes from the received address, we get the table of functions that are run in the program initialization phase. The program `dd(1)` can be used to dump the data. With the programs `dd(1)`, `od(1)`, and `nl(1)`, you can get valid function addresses. After inspecting each address with the `objdump(1)` disassembler, it can be concluded that the second address is responsible for running the `Test::Test()` constructor.

# PIC Code
Forced compilation to a form without PIC code is possible with the argument `-mcmodel=large`:

```
s3jk1@hopper:~/GitHub/Runtime-on-GNU-Linux-systems$ gcc test.c -o test-nopic -mcmodel=large -O0
s3jk1@hopper:~/GitHub/Runtime-on-GNU-Linux-systems$ gcc test.c -o test-pic -fPIC -O0
```

The compilation results in two executables: test-nopic, which does not contain PIC code, and test-pic with PIC code. You can disassembly this with `objdump()` function.
