add_library(runtime SHARED
  GlobalContext.cpp
  hook.c

  process.cpp
  query.cpp

  platform/unix/dlhook.cpp

  test_main.cpp
)

target_link_libraries(runtime PRIVATE -lcapstone)
