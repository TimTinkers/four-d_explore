// Imports.
#include "terrain.h"
#include <algorithm>
#include <math.h>
#include "perlin.h"
#include "tetrahedron.h"
#include <iostream>
#include "openSimplex\open-simplex-noise.h"

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
Terrain::Chunk::Chunk(glm::ivec4 c, float persistance, float frequency, int noiseMode)
	: dimensions_(c) {
	int xSize = dimensions_.x;
	int ySize = dimensions_.y;
	int zSize = dimensions_.z;
	int wSize = dimensions_.w;
	for (int x = 0; x < xSize; ++x) {
		for (int y = 0; y < ySize; ++y) {
			for (int z = 0; z < zSize; ++z) {
				for (int w = 0; w < wSize; ++w) {
					glm::ivec4 n_coord(x, y, z, w);

					// If the noise mode is set to zero, use our implementation of Perlin noise.
					float val = 0;
					if (noiseMode == 0) {
						val = Perlin::octave(x, y, z, w, persistance, frequency);
					} else if (noiseMode == 1) {

						// If the noise mode is set to one, use the use smcameron's open-simplex-noise implementation.
						int FEATURE_SIZE = pow(xSize * xSize + ySize * ySize + zSize * zSize + wSize * wSize, 0.25);
						struct osn_context *ctx;
						int seed = (int)(77374 + 7 * persistance + 13 * frequency);
						open_simplex_noise(seed, &ctx);
						val = open_simplex_noise4(ctx, (double)x / FEATURE_SIZE, (double)y / FEATURE_SIZE,
							(double)z / FEATURE_SIZE, (double)w / FEATURE_SIZE);
					}
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