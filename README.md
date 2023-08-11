# LiveMod
A library to hook at run-time an executable.

> **Note**
> Only Linux on x86_64 is supported so far, but it's possible to port the library to other platforms.

## Using it
### Building
In the project root:
```
mkdir build
cd build
cmake ..
cmake --build .
```
### Running
The runtime library is `build/runtime/libruntime.so`.

To hook an executable, execute it as follow:
```
LD_PRELOAD=<path-to-LiveMod>/build/runtime/libruntime.so <executable>
```

# Ideas

### Overriding dynamically linked symbols
Preloading a shared library by using `LD_PRELOAD` allows to replace any dynamically linked symbol with a value of our choice.

### Hooking/Patching
Trampoline hooks allow to replace statically linked functions with a function of our choice.
This is done at run-time.

### Getting the symbols
* `objdump`, allows getting a list of symbols in the executable file.
* `/proc/self/maps` allows tranforming the symbol in the object file to a run-time address
* `dlsym(name, NULL)` allows getting the address of a *dynamic* symbol at run-time
