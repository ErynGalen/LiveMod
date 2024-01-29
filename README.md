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
The launcher is `/build/src/livemod`
The runtime library is `build/src/runtime/libruntime.so`.

To hook a command, execute it as follow:
```
<livemod> <libruntime> command [arg1 arg2 ...]
```
where `<livemod>` is the path to the `livemod` executable, and  `<libruntime>` the path to `libruntime.so`.

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


### Windows
[https://github.com/sengelha/win32-ldpreload](https://github.com/sengelha/win32-ldpreload)
[https://en.wikipedia.org/wiki/DLL_injection](https://en.wikipedia.org/wiki/DLL_injection)
