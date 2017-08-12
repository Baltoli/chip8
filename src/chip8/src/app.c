#include "interpreter.h"

#include <GLFW/glfw3.h>

#include <stdio.h>

void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

int main(int argc, char **argv)
{ 
  glfwSetErrorCallback(error_callback);

  if(!glfwInit()) {
    return 1;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow* window = glfwCreateWindow(640, 480, "My Title", NULL, NULL);
  if(!window) {
    return 2;
  }
  glfwMakeContextCurrent(window);

  while(true){glfwPollEvents();}

  struct c8_interpreter *in = new_interpreter();
  in->cpu.memory[0x200] = 0x6081;
  in->cpu.memory[0x201] = 0x800E;

  run(in);
  dump_state(in);

  glfwTerminate();
  return 0;
}
