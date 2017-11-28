#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "GLFW/glfw3.h"

#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <streambuf>
#include <sstream>

#include "matrix.h"
#include "app.h"

// Before we really dove into blockchain work.
int main() {
  /* WIP dynamically-reading shaders.
  // Import the fragment shader to use.
  std::ifstream t("shaders/example.frag");
  std::stringstream buffer;
  buffer << t.rdbuf();
  std::cout << buffer.str().c_str();
  */

  mat5::perspective(30, 0.75, 0.75, 1, 20).Print();
  mat5::lookAt(glm::vec4(0,0,0,-1), glm::vec4(0,0,0,0), glm::vec4(0,1,0,0), glm::vec4(1,0,0,0)).Print();

  std::shared_ptr<App> app_ptr(new App());
  app_ptr->init();
  app_ptr->run();
  return 0;
}
