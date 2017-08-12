add_executable(c8i
  src/app.c
  src/interpreter.c
)

target_link_libraries(c8i glfw)
target_compile_options(c8i PRIVATE -Wall -Wextra -pedantic)
