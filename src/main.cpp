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
<<<<<<< HEAD
  glm::mat4 mat = glm::perspective(30.0f, 1.0f, 2.0f, 10.0f);
  for (int i = 0; i < 4; ++i) {
    for (int j = 0; j < 4; ++j) {
      std::cout << mat[i][j] << " ";
    }
    std::cout << "\n";
  }
  mat = glm::lookAt(glm::vec3(0, 0, 3), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
  for (int i = 0; i < 4; ++i) {
    for (int j = 0; j < 4; ++j) {
      std::cout << mat[i][j] << " ";
    }
    std::cout << "\n";
  }

  std::shared_ptr<App> app_ptr(new App());

  printf("Initializing...\n");
=======

  mat5::perspective(30, 0.75, 0.75, 1, 20).Print();
  mat5::lookAt(glm::vec4(0,0,0,-1), glm::vec4(0,0,0,0), glm::vec4(0,1,0,0), glm::vec4(1,0,0,0)).Print();

  std::shared_ptr<App> app_ptr(new App());
>>>>>>> 0de48caba3c8d8457cc2275015ff902e2b94cf95
  app_ptr->init();
  printf("Initialized. Running...\n");
  app_ptr->run();
  printf("Run function completed.\n");

  return 0;
}
