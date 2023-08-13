add_executable(dlsym_test
  dlsym.c
)

add_executable(disas
  disas.cpp
)
target_link_libraries(disas -lLLVM)

add_executable(exec
  exec.cpp
)
