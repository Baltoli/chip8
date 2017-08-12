#include "interpreter.h"

#include <GLFW/glfw3.h>

#include <stdio.h>
#include <stdlib.h>

void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

GLFWwindow *init_graphics(void)
{
  glfwSetErrorCallback(error_callback);

  if(!glfwInit()) {
    return NULL;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow* window = glfwCreateWindow(640, 480, "My Title", NULL, NULL);

  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);

  return window;
}

int main(int argc, char **argv)
{ 
  GLFWwindow *wind = init_graphics();
  if(!wind) {
    glfwTerminate();
    return 1;
  }

  struct c8_interpreter *in = new_interpreter();
  run(in);
  dump_state(in);

  while (!glfwWindowShouldClose(wind)) {
    glfwSwapBuffers(wind);
    glfwPollEvents();
  }

  glfwDestroyWindow(wind);
  glfwTerminate();
  return 0;
}
