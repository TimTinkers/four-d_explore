// Imports.
#include "terrain.h"

#include <algorithm>
#include <math.h>

#include "perlin.h"
#include "tetrahedron.h"

#include <iostream>

/**
 *	Initialize a block at the given integer coordinates.
 */
Terrain::Block::Block(glm::ivec4 c, int t) : pos_(c), type_(t) {}

glm::ivec4 Terrain::Block::GetPos() { return pos_; }

int Terrain::Block::GetType() { return type_; }

/**
 *	Generate a single 4D chunk of terrain using Perlin noise.
 *	The chunk is rooted at the given coordinates.
 */
Terrain::Chunk::Chunk(glm::ivec4 c, float persistance, float frequency) : dimensions_(c) {
	int xSize = dimensions_.x;
	int ySize = dimensions_.y;
	int zSize = dimensions_.z;
	int wSize = dimensions_.w;
	for (int x = 0; x < xSize; ++x) {
		for (int y = 0; y < ySize; ++y) {
			for (int z = 0; z < zSize; ++z) {
				for (int w = 0; w < wSize; ++w) {
					glm::ivec4 n_coord(x, y, z, w);
					float val = Perlin::octave(x, y, z, w, persistance, frequency);
					val += std::max(0.0,
						5.0 - sqrt(pow(y, 2.0) + pow(z, 2.0) + pow(w, 2.0)));
					if (val > 0) {
						blocks_.emplace(n_coord, Terrain::Block(n_coord, 1));
					} else {
						blocks_.emplace(n_coord, Terrain::Block(n_coord, 0));
					}
				}
			}
		}
	}
}

Terrain::Block* Terrain::Chunk::GetBlock(glm::ivec4 c) {
	if (blocks_.count(c) > 0) {
		return &blocks_.at(c);
	}
}

std::vector<Terrain::Block*> Terrain::Chunk::GetAllBlocks() {
	std::vector<Terrain::Block*> outputBlocks;
	int xSize = dimensions_.x;
	int ySize = dimensions_.y;
	int zSize = dimensions_.z;
	int wSize = dimensions_.w;
	for (int x = 0; x < xSize; ++x) {
		for (int y = 0; y < ySize; ++y) {
			for (int z = 0; z < zSize; ++z) {
				for (int w = 0; w < wSize; ++w) {
					glm::ivec4 n_coord(x, y, z, w);
					outputBlocks.push_back(&blocks_.at(n_coord));
				}
			}
		}
	}
	return outputBlocks;
}