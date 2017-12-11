// Imports.
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

using namespace std;

// Main entry-point of the visualizer.
int main(int argc, char* argv[]) {

	if (argc < 4) {
		cout << "Use: " << argv[0] 
			 << " <Width> <Height> <Scene File | PERLIN | OPENSIMPLEX> [persistence] [frequency] [x size] [y size] [z size] [w size]\n";
	} else {

		// Retrieve the window dimensions.
		int width = atoi(argv[1]);
		int height = atoi(argv[2]);

		// Retrieve the scene to render.
		char* scene = argv[3];
		if (!strcmp(scene, "PERLIN")) {

			// If the user requests perlin noise, create a new scene for them.
			cout << "Generating perlin noise for scene.\n";

			// Retrieve various perlin parameters if they were given.
			float persistence = 0.5f;
			if (argc >= 5) {
				persistence = atof(argv[4]);
			}
			float frequency = 2.0f;
			if (argc >= 6) {
				frequency = atof(argv[5]);
			}
			int xSize = 12;
			if (argc >= 7) {
				xSize = atoi(argv[6]);
			}
			int ySize = 12;
			if (argc >= 8) {
				ySize = atoi(argv[7]);
			}
			int zSize = 12;
			if (argc >= 9) {
				zSize = atoi(argv[8]);
			}
			int wSize = 12;
			if (argc >= 10) {
				wSize = atoi(argv[9]);
			}

			// Generate the terrain.
			Terrain::Chunk c(glm::ivec4(xSize, ySize, wSize, zSize), persistence, frequency, 0);
			std::vector<Terrain::Block*> blocks = c.GetAllBlocks();

			// Initialize the app.
			std::shared_ptr<App> app_ptr(new App(width, height, blocks));
			app_ptr->init();
			printf("Initialized. Running...\n");

			// Run the app.
			app_ptr->run();
			printf("Run function completed.\n");
		} else if (!strcmp(scene, "OPENSIMPLEX")) {

			// If the user requests perlin noise, create a new scene for them.
			cout << "Generating open simplex noise for scene.\n";

			// Retrieve various perlin parameters if they were given.
			float persistence = 0.5f;
			if (argc >= 5) {
				persistence = atof(argv[4]);
			}
			float frequency = 2.0f;
			if (argc >= 6) {
				frequency = atof(argv[5]);
			}
			int xSize = 12;
			if (argc >= 7) {
				xSize = atoi(argv[6]);
			}
			int ySize = 12;
			if (argc >= 8) {
				ySize = atoi(argv[7]);
			}
			int zSize = 12;
			if (argc >= 9) {
				zSize = atoi(argv[8]);
			}
			int wSize = 12;
			if (argc >= 10) {
				wSize = atoi(argv[9]);
			}

			// Generate the terrain.
			Terrain::Chunk c(glm::ivec4(xSize, ySize, wSize, zSize), persistence, frequency, 1);
			std::vector<Terrain::Block*> blocks = c.GetAllBlocks();

			// Initialize the app.
			std::shared_ptr<App> app_ptr(new App(width, height, blocks));
			app_ptr->init();
			printf("Initialized. Running...\n");

			// Run the app.
			app_ptr->run();
			printf("Run function completed.\n");
		} else {

			// The user specified a file to read existing data from.
			ifstream meshData(scene);
			if (!meshData.is_open()) {
				cout << "Could not read \"" << scene << "\"\n";
			}  else {
				
				// Parse the terrain.
				std::vector<Terrain::Block*> blocks;
				int x, y, z, w;
				while (meshData >> x >> y >> w >> z) {
					blocks.push_back(new Terrain::Block(glm::ivec4(x, y, z, w), 1));
				}

				// Initialize the app.
				std::shared_ptr<App> app_ptr(new App(width, height, blocks));
				app_ptr->init();
				printf("Initialized. Running...\n");

				// Run the app.
				app_ptr->run();
				printf("Run function completed.\n");
			}
		}
	}
	return 0;
}
