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
* `LD_AUDIT=<path>` - points to a shared library that implements the `rtld-audit(7)` interface.

# LD_PRELOAD
It allows the selected library to be loaded into the process memory space early, before any dependent library declared in the `.dynamic` section is loaded. Thanks to this, the selected library has priority in determining the addresses of symbols imported by the application, and thus - it is possible to "override" functions from other libraries with your own implementations.

In the `shadow.c` program we have written, one of many possible scenarios is presented using the variable `LD_PRELOAD`. It shows overriding the `connect(2)` function by creating your own minimal library with its new implementation. The new function `connect(2)` uses the original function from the *libc* library, creating from the library a simple sniffer of addresses that the selected palette tries to connect to.

Compilation is done with the command:

```
$ gcc shadow.c -o libshadow.so -shared -fPIC -std=c11 -ldl
```

The compiler should generate a library called libshadow.so, the name of which (together with the path) should be placed in the content of the variable `LD_PRELOAD`. An example of a call may be an attempt to run the program `wget(1)` to download the home page of wp.pl. However, the call fails with an error `ECONNREFUSED`.

```
$ LD_PRELOAD=./libshadow.so wget wp.pl
```

Trying to download the home page of any other site will be fine.

This is what libshadow.so early loading works. The boot loader loaded libshadow.so before loading libc.so. During the symbol loading process, when the loader tries to find the address of the `connect` function, it will find it in libshadow.so. The address for the implementation of the `connect` function in libshadow.so instead of libc.so will be written to the GOT. Each call to the `connect` function using the PLT table (that is, each compiler-generated function call) will pass control to libshadow.so.

The first step by the libshadow.so `connect` function is to try to determine the libc.so base address in the current process's address space using the `dlopen` function. Note that if the argument of `dlopen` is the name of a library that is already loaded in the process, the function `dlopen` only returns its current base address, rather than creating a second instance of it. Once the base address of this library has been located, the address of the `connect` function is located in libc.so using the `dlsym` function. When this location is successful, the found address of the original `connect` function is written to the global variable `orig_connect`.

The next step is to check if the `connect` function was called with an argument specifying an attempt to connect to the TCP/IPv4 protocol address. This check is done by comparing the `sa_family` field of the `sockaddr` structure with the constant `AF_INET` and the `address_len` argument containing information about the size of the passed `sockaddr` structure with the various` sockaddr_in` structure, because this is exactly the structure that should be passed to the `connect` when the `sa_family` field equals `AF_INET`.

When the above condition is met, it is possible to read the target IPv4 address by reading the `sin_addr.s_addr` field of the `sockaddr_in` structure that was passed by the program. The value of this field is in *big-endian* encoding, so it needs to be converted to system encoding by calling the `ntohl` function. The address obtained in this way is stored in a variable of type `uint32_t` and can be compared with the selected IPv4 address built using the macro `MAKE_IPv4`. If the addresses match, the libshadow.so implementation of `connect` returns the error -`ECONNREFUSED`, setting the global variable `errno` to the same error code. If, however, the IPv4 address is different, then the original `connect` function from libc.so is called, resulting in a good network connection to the selected server. The data transfer should therefore proceed without much problem.

The `LD_PRELOAD` mechanism allows for the creation of a fairly effective isolation of the selected program from system functions, allowing control over information transferred from the operating system. However, it is not suitable for implementing the 'sandboxing' application isolation technique because any application is able to call system functions directly. The mechanism `LD_PRELOAD` must not prevent such a call from being made. This is in turn possible using the `ptrace` function.

# LD_AUDIT

The dependency graph between dependent libraries can sometimes be very complicated. With a non-standard system configuration or when a given program uses a number of libraries as dependencies and each of these libraries has its own dependencies, the reason for loading a given library is not always obvious. Problems with the process of locating dependent libraries or the desire to control this process can be solved by using a mechanism called `rtld-audit`.

Calling the audit mechanism can be done in three ways:

1. defining the `LD_AUDIT` environment variable, which takes the path to the library implementing the correct interface accepted by `rtld-audit()`
2. using the bootloader `--audit` argument, specifying the path to the library as the argument
3. using the `DT_AUDIT` attribute in the `.dynamic` section; this attribute should contain the path to the library which is to act as the auditor.

# Code injection

Compilation:

```
$ gcc test2.c -o test2
```

The program is limited to reading one character from the * stdin * stream, displaying its ASCII code, and then terminating it immediately. After entering one character and confirming with the * Enter * key, the program displays the ASCII code of the entered character and finishes its operation:

```
$ ./test
A
65
```

The demonstration of modifying the code will consist in changing the program in such a way that it does not end after reading one character, but runs indefinitely, until stopped by pressing the key combination CTRL + C. To do this, you will need the program *test* itself and a short script in the selected language (Python for me), which will execute the commands to write the appropriate code bytes to the right places. To write this script, the `ndisasm` program, which is part of the nasm package, will be used.

Usually during work in which the memory of other programs is changed, for various reasons it is required to run them multiple times; either because the program lifetime is just very short, or because when you change the memory of a foreign program, you can't do it perfectly. Virtually every time the target program throws an exception `SIGSEGV`,` SIGILL`, or the like, because of an error in the code by manual modification of it. To make your work easier, it's best to write a script that automates the process identification and displaying the PID number to the standard output.

The `getpid.py` script aims to search all directories in `/proc` and find one where a symbolic exe link points to some file in the same directory as the script. When running `test2` in another console, calling the script should display the PID of `test2`.

**1st terminal**
```
$ ./test2
```

**2nd terminal**
```
$ sudo python3 getpid.py
```

The `procwrite.py` script uses the `getpid.py` script you wrote earlier to identify the *test* process, so you don't have to hard-write the process id.

Saving the memory to the selected process is a matter of opening the mem file and saving the data to the selected address. Any restrictions on memory access rights will be ignored by the *procfs* driver itself, which will call the appropriate kernel functions to perform the write operation to the selected address in the memory of the selected foreign process.

Launching the program should be performed as the root user, because it is the fastest method of obtaining the appropriate access rights to the mem file:
```
$ sudo python3 procwrite.py
```

It is also possible to verify the written data with the command line, which directly reads the code from memory and disassembles it using the `ndisasm` tool:

```
# dd if=/proc/`python3 getpid.py`/mem bs=1 skip=$((0x4005b0)) count=51 2>/dev/null | ndisasm -b 64 -
```

Make sure you have the `nasm` package installed on your computer.

