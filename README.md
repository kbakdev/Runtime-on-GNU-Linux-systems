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
$ gcc test.c -o test-nopic -mcmodel=large -O0
$ gcc test.c -o test-pic -fPIC -O0
```

The compilation results in two executables: test-nopic, which does not contain PIC code, and test-pic with PIC code. You can disassembly this with `objdump()` function.

# GOT and PLT tables
The operation of the GOT is related to the PLT mechanism.
Compilation of the program is possible via the command line:
```
$ gcc readdir.c -o readdir
```

The program uses external symbols, implemented in another library, which will be marked as a dependent library. Symbols such as `opendir(3)`, `closedir(3)`, and `readdir(3)` will have to be localized by the loader during the program initialization process.

During the compilation process, the compiler is able to determine the number of symbols imported from the dependent libraries. Therefore, it can reserve a place in the GOT for each such symbol. The initial values, i.e. those saved in the file, will be replaced with correct addresses in the memory of the imported functions while the program is running. The GOT is located at the address indicated by the symbol `_GLOBAL_OFFSET_TABLE_`, as seen by the symbol name and address dump tool `nm(1)`.

```
$ nm readdir | grep GLOBAL_OFFSET_TABLE
```

# Environment Variables

The operation of the loader can be influenced by a set of options activated by setting the appropriate environment variables. The most frequently used variables are:
* `LD_PRELOAD=<path>` - sets the library search path to the selected directory or directory list (separated by a colon).

# LD_PRELOAD
It allows the selected library to be loaded into the process memory space early, before any dependent library declared in the `.dynamic` section is loaded. Thanks to this, the selected library has priority in determining the addresses of symbols imported by the application, and thus - it is possible to "override" functions from other libraries with your own implementations.

In the `shadow.c` program we have written, one of many possible scenarios is presented using the variable `LD_PRELOAD`. It shows overriding the `connect(2)` function by creating your own minimal library with its new implementation. The new function `connect(2)` uses the original function from the *libc* library, creating from the library a simple sniffer of addresses that the selected palette tries to connect to.