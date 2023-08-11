add_executable(dlsym_test
  dlsym.c
)
set_target_properties(dlsym_test PROPERTIES
  LINK_FLAGS -rdynamic
)

add_executable(disas
  disas.cpp
)
target_link_libraries(disas -lLLVM)

add_executable(exec
  exec.c
)
