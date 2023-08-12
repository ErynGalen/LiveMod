add_library(runtime SHARED
  src/GlobalContext.cpp
  src/Hook.cpp

  src/process.cpp
  src/query.cpp

  src/asm/asm.cpp
  src/asm/GlobalLLVM.cpp

  src/platform/unix/dlhook.cpp

  src/test_main.cpp
)

target_link_libraries(runtime PRIVATE -lLLVM)
