add_executable(c8i
  src/app.c
  src/interpreter.c
)

target_link_libraries(c8i ${SDL2_LIBRARIES})
target_include_directories(c8i PRIVATE ${SDL2_INCLUDE_DIRS})
target_compile_options(c8i PRIVATE -Wall -Wextra -pedantic)
