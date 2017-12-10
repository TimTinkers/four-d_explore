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

#include "terrain.h"
#include "perlin.h"

int main() {
	
	// Test Perlin noise generation.
	/*
	bool hasMin = false;
	bool hasMax = false;
	float minValue = 0;
	float maxValue = 0;
	for (int x = 0; x < 16 * 4; ++x) {
		printf("x-layer: %d\n", x);
		for (int y = 0; y < 16 * 4; ++y) {
			for (int z = 0; z < 16 * 4; ++z) {
				for (int w = 0; w < 16 * 4; ++w) {
					float val = Perlin::octave(x, y, z, w);
					if (!hasMin || val < minValue) {
						minValue = val;
						hasMin = true;
					}
					if (!hasMax || val > maxValue) {
						maxValue = val;
						hasMax = true;
					}
				}
			}
		}
	}
	printf("Perlin min: %f, max: %f\n", minValue, maxValue);
	while (true) {}
	*/

	Terrain::Chunk c(glm::ivec4(0));
	std::vector<Terrain::Block*> blocks = c.GetAllBlocks();

	for (int i = 0; i < blocks.size(); i++) {
		Terrain::Block* block = blocks.at(i);
		glm::ivec4 blockPos = block->GetPos();
		if (block->GetType() > 0) {
			printf("Block pos: %d, %d, %d, %d\n", blockPos.x, blockPos.y, blockPos.z, blockPos.w);
		} else {
			printf("Empty pos: %d, %d, %d, %d\n", blockPos.x, blockPos.y, blockPos.z, blockPos.w);
		}
	}

	std::shared_ptr<App> app_ptr(new App(blocks));
	app_ptr->init();
	printf("Initialized. Running...\n");
	app_ptr->run();
	printf("Run function completed.\n");

	return 0;
}
