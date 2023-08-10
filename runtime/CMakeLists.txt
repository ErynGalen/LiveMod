add_library(runtime SHARED
  src/dlhook.cpp
  src/query.cpp
  src/GlobalContext.cpp
  src/Hook.cpp

  src/asm/asm.cpp
)
