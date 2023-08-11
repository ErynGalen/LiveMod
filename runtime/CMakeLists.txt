add_library(runtime SHARED
  src/dlhook.cpp
  src/process.cpp
  src/query.cpp
  src/GlobalContext.cpp
  src/Hook.cpp

  src/asm/asm.cpp
  src/asm/GlobalLLVM.cpp
)
target_link_libraries(runtime PRIVATE -lLLVM)
