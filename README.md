# Ideas

### Overriding dynamically linked symbols
Preloading a shared library by using `LD_PRELOAD` allows to replace any dynamically linked symbol with a value of our choice.

### Hooking/Patching
Trampoline hooks allow to replace statically linked functions with a function of our choice.
This is done at run-time.

### Getting the symbols
* `nm`, `objdump`, `readelf` allow getting a list of symbols in an ELF file.
* `dlsym(name, NULL)` allow getting the address of a *dynamic* symbol at run-time
* `elfhacks` (library) allow getting the address of any symbol at run-time, at the cost of reading ELF files.
