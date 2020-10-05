# Early program initialization

`main()` is not the first function your program runs. The ELF file includes additional mechanisms to perform an early initialization that can be run before `main()` gets control: using the `DT_INIT` and `DT_INITARRAY` keys.

The gcc compiler supports putting its own functions into the `DT_INITARRAY` keys through the *constructor* attribute.
