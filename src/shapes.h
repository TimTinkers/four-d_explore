#pragma once
#include <vector>
#include <glm/vec4.hpp>

/**
 *	This file contains mesh data for some interesting testing scenarios.
 */

// This scenario demonstrates an impenetrable 3D wall which can be circumnavigated by moving "across" it.
std::vector<glm::vec4> WALL_SCENARIO = {

	// The wall.
	glm::vec4(0, 0, 0, 0), glm::vec4(1, 0, 0, 0), glm::vec4(2, 0, 0, 0), glm::vec4(3, 0, 0, 0), glm::vec4(4, 0, 0, 0),
	glm::vec4(0, 1, 0, 0), glm::vec4(1, 1, 0, 0), glm::vec4(2, 1, 0, 0), glm::vec4(3, 1, 0, 0), glm::vec4(4, 1, 0, 0),
	glm::vec4(0, 2, 0, 0), glm::vec4(1, 2, 0, 0), glm::vec4(2, 2, 0, 0), glm::vec4(3, 2, 0, 0), glm::vec4(4, 2, 0, 0),
	glm::vec4(0, 3, 0, 0), glm::vec4(1, 3, 0, 0), glm::vec4(2, 3, 0, 0), glm::vec4(3, 3, 0, 0), glm::vec4(4, 3, 0, 0),
	glm::vec4(0, 4, 0, 0), glm::vec4(1, 4, 0, 0), glm::vec4(2, 4, 0, 0), glm::vec4(3, 4, 0, 0), glm::vec4(4, 4, 0, 0),

	// The wall shifted across the fourth axis, with a hole in it.
	glm::vec4(0, 0, 4, 0), glm::vec4(1, 0, 4, 0), glm::vec4(2, 0, 4, 0), glm::vec4(3, 0, 4, 0), glm::vec4(4, 0, 4, 0),
	glm::vec4(0, 1, 4, 0),																		glm::vec4(4, 1, 4, 0),
	glm::vec4(0, 2, 4, 0),																		glm::vec4(4, 2, 4, 0),
	glm::vec4(0, 3, 4, 0),																		glm::vec4(4, 3, 4, 0),
	glm::vec4(0, 4, 4, 0), glm::vec4(1, 4, 4, 0), glm::vec4(2, 4, 4, 0), glm::vec4(3, 4, 4, 0), glm::vec4(4, 4, 4, 0),

	// Put something to see behind the wall.
	glm::vec4(2, 2, 0, 7),
};

// This scenario demonstrates a large block of hypercubes.
std::vector<glm::vec4> BLOCK_SCENARIO = {

	glm::vec4(0, 0, 0, 0), glm::vec4(1, 0, 0, 0), glm::vec4(2, 0, 0, 0), glm::vec4(3, 0, 0, 0), glm::vec4(4, 0, 0, 0),
	glm::vec4(0, 1, 0, 0), glm::vec4(1, 1, 0, 0), glm::vec4(2, 1, 0, 0), glm::vec4(3, 1, 0, 0), glm::vec4(4, 1, 0, 0),
	glm::vec4(0, 2, 0, 0), glm::vec4(1, 2, 0, 0), glm::vec4(2, 2, 0, 0), glm::vec4(3, 2, 0, 0), glm::vec4(4, 2, 0, 0),
	glm::vec4(0, 3, 0, 0), glm::vec4(1, 3, 0, 0), glm::vec4(2, 3, 0, 0), glm::vec4(3, 3, 0, 0), glm::vec4(4, 3, 0, 0),
	glm::vec4(0, 4, 0, 0), glm::vec4(1, 4, 0, 0), glm::vec4(2, 4, 0, 0), glm::vec4(3, 4, 0, 0), glm::vec4(4, 4, 0, 0),

	glm::vec4(0, 0, 1, 0), glm::vec4(1, 0, 1, 0), glm::vec4(2, 0, 1, 0), glm::vec4(3, 0, 1, 0), glm::vec4(4, 0, 1, 0),
	glm::vec4(0, 1, 1, 0), glm::vec4(1, 1, 1, 0), glm::vec4(2, 1, 1, 0), glm::vec4(3, 1, 1, 0), glm::vec4(4, 1, 1, 0),
	glm::vec4(0, 2, 1, 0), glm::vec4(1, 2, 1, 0), glm::vec4(2, 2, 1, 0), glm::vec4(3, 2, 1, 0), glm::vec4(4, 2, 1, 0),
	glm::vec4(0, 3, 1, 0), glm::vec4(1, 3, 1, 0), glm::vec4(2, 3, 1, 0), glm::vec4(3, 3, 1, 0), glm::vec4(4, 3, 1, 0),
	glm::vec4(0, 4, 1, 0), glm::vec4(1, 4, 1, 0), glm::vec4(2, 4, 1, 0), glm::vec4(3, 4, 1, 0), glm::vec4(4, 4, 1, 0),

	glm::vec4(0, 0, 2, 0), glm::vec4(1, 0, 2, 0), glm::vec4(2, 0, 2, 0), glm::vec4(3, 0, 2, 0), glm::vec4(4, 0, 2, 0),
	glm::vec4(0, 1, 2, 0), glm::vec4(1, 1, 2, 0), glm::vec4(2, 1, 2, 0), glm::vec4(3, 1, 2, 0), glm::vec4(4, 1, 2, 0),
	glm::vec4(0, 2, 2, 0), glm::vec4(1, 2, 2, 0), glm::vec4(2, 2, 2, 0), glm::vec4(3, 2, 2, 0), glm::vec4(4, 2, 2, 0),
	glm::vec4(0, 3, 2, 0), glm::vec4(1, 3, 2, 0), glm::vec4(2, 3, 2, 0), glm::vec4(3, 3, 2, 0), glm::vec4(4, 3, 2, 0),
	glm::vec4(0, 4, 2, 0), glm::vec4(1, 4, 2, 0), glm::vec4(2, 4, 2, 0), glm::vec4(3, 4, 2, 0), glm::vec4(4, 4, 2, 0),

	glm::vec4(0, 0, 3, 0), glm::vec4(1, 0, 3, 0), glm::vec4(2, 0, 3, 0), glm::vec4(3, 0, 3, 0), glm::vec4(4, 0, 3, 0),
	glm::vec4(0, 1, 3, 0), glm::vec4(1, 1, 3, 0), glm::vec4(2, 1, 3, 0), glm::vec4(3, 1, 3, 0), glm::vec4(4, 1, 3, 0),
	glm::vec4(0, 2, 3, 0), glm::vec4(1, 2, 3, 0), glm::vec4(2, 2, 3, 0), glm::vec4(3, 2, 3, 0), glm::vec4(4, 2, 3, 0),
	glm::vec4(0, 3, 3, 0), glm::vec4(1, 3, 3, 0), glm::vec4(2, 3, 3, 0), glm::vec4(3, 3, 3, 0), glm::vec4(4, 3, 3, 0),
	glm::vec4(0, 4, 3, 0), glm::vec4(1, 4, 3, 0), glm::vec4(2, 4, 3, 0), glm::vec4(3, 4, 3, 0), glm::vec4(4, 4, 3, 0),

	glm::vec4(0, 0, 4, 0), glm::vec4(1, 0, 4, 0), glm::vec4(2, 0, 4, 0), glm::vec4(3, 0, 4, 0), glm::vec4(4, 0, 4, 0),
	glm::vec4(0, 1, 4, 0), glm::vec4(1, 1, 4, 0), glm::vec4(2, 1, 4, 0), glm::vec4(3, 1, 4, 0), glm::vec4(4, 1, 4, 0),
	glm::vec4(0, 2, 4, 0), glm::vec4(1, 2, 4, 0), glm::vec4(2, 2, 4, 0), glm::vec4(3, 2, 4, 0), glm::vec4(4, 2, 4, 0),
	glm::vec4(0, 3, 4, 0), glm::vec4(1, 3, 4, 0), glm::vec4(2, 3, 4, 0), glm::vec4(3, 3, 4, 0), glm::vec4(4, 3, 4, 0),
	glm::vec4(0, 4, 4, 0), glm::vec4(1, 4, 4, 0), glm::vec4(2, 4, 4, 0), glm::vec4(3, 4, 4, 0), glm::vec4(4, 4, 4, 0),
};
