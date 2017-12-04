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

int main() {
	mat5::perspective(30, 0.75, 0.75, 1, 20).Print();
	mat5::lookAt(glm::vec4(0,0,0,-1), glm::vec4(0,0,0,0), glm::vec4(0,1,0,0), glm::vec4(1,0,0,0)).Print();

	std::shared_ptr<App> app_ptr(new App());

	app_ptr->init();
	printf("Initialized. Running...\n");
	app_ptr->run();
	printf("Run function completed.\n");

	return 0;
}