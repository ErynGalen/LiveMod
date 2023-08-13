add_library(runtime SHARED
  GlobalContext.cpp
  Hook.cpp

  process.cpp
  query.cpp

  asm/asm.cpp
  asm/GlobalLLVM.cpp

  platform/unix/dlhook.cpp

  test_main.cpp
)

target_link_libraries(runtime PRIVATE -lLLVM)
