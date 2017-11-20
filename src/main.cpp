#include "app.h"
#include <memory>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <fstream>
#include <iostream>
#include <string>
#include <streambuf>
#include <sstream>

// Before we really dove into blockchain work.
int main() {

	/* WIP dynamically-reading shaders.
	// Import the fragment shader to use.
	std::ifstream t("shaders/example.frag");
	std::stringstream buffer;
	buffer << t.rdbuf();
	std::cout << buffer.str().c_str();
	*/

  std::shared_ptr<App> app_ptr(new App());
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
  app_ptr->init();
  app_ptr->run();
  return 0;
}
