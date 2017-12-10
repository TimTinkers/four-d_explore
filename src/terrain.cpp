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
Terrain::Chunk::Chunk(glm::ivec4 c) : ref_(c) {
  for (int x = 0; x < CHUNK_SIZE; ++x) {
    for (int y = 0; y < CHUNK_SIZE; ++y) {
      for (int z = 0; z < CHUNK_SIZE; ++z) {
        for (int w = 0; w < CHUNK_SIZE; ++w) {
          glm::ivec4 n_coord(x, y, z, w);
          n_coord += ref_;
          float val = Perlin::octave(x, y, z, w);
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
  for (int x = 0; x < CHUNK_SIZE; ++x) {
    for (int y = 0; y < CHUNK_SIZE; ++y) {
      for (int z = 0; z < CHUNK_SIZE; ++z) {
        for (int w = 0; w < CHUNK_SIZE; ++w) {
          glm::ivec4 n_coord(x, y, z, w);
          n_coord += ref_;
          outputBlocks.push_back(&blocks_.at(n_coord));
        }
      }
    }
  }
  return outputBlocks;
}

Terrain::Block* Terrain::GetBlock(glm::ivec4 c) {
  glm::ivec4 ref =
      c - glm::ivec4(c[0] - c[0] % CHUNK_SIZE, c[1] - c[1] % CHUNK_SIZE,
                     c[2] - c[2] % CHUNK_SIZE, c[3] - c[3] % CHUNK_SIZE);
  if (chunks_.count(ref) > 0) {
    return chunks_.at(ref).GetBlock(c);
  } else {
    return nullptr;
  }
}

void Terrain::GenChunk(glm::ivec4 ref) {
  if (chunks_.count(ref) > 0) {
    chunks_.emplace(ref, ref);
  }
}

std::vector<Tetrahedron> Terrain::Block::GetTets() {
  std::vector<Tetrahedron> tets;
  std::vector<glm::vec4> corners;
  corners.emplace_back(glm::vec4(pos_) + glm::vec4(-0.5f, -0.5f, -0.5f, -0.5f));
  corners.emplace_back(glm::vec4(pos_) + glm::vec4(-0.5f, -0.5f, -0.5f, 0.5f));
  corners.emplace_back(glm::vec4(pos_) + glm::vec4(-0.5f, -0.5f, 0.5f, 0.5f));
  corners.emplace_back(glm::vec4(pos_) + glm::vec4(-0.5f, -0.5f, 0.5f, -0.5f));
  corners.emplace_back(glm::vec4(pos_) + glm::vec4(-0.5f, 0.5f, -0.5f, -0.5f));
  corners.emplace_back(glm::vec4(pos_) + glm::vec4(-0.5f, 0.5f, -0.5f, 0.5f));
  corners.emplace_back(glm::vec4(pos_) + glm::vec4(-0.5f, 0.5f, 0.5f, 0.5f));
  corners.emplace_back(glm::vec4(pos_) + glm::vec4(-0.5f, 0.5f, 0.5f, -0.5f));
  corners.emplace_back(glm::vec4(pos_) + glm::vec4(0.5f, -0.5f, -0.5f, -0.5f));
  corners.emplace_back(glm::vec4(pos_) + glm::vec4(0.5f, -0.5f, -0.5f, 0.5f));
  corners.emplace_back(glm::vec4(pos_) + glm::vec4(0.5f, -0.5f, 0.5f, 0.5f));
  corners.emplace_back(glm::vec4(pos_) + glm::vec4(0.5f, -0.5f, 0.5f, -0.5f));
  corners.emplace_back(glm::vec4(pos_) + glm::vec4(0.5f, 0.5f, -0.5f, -0.5f));
  corners.emplace_back(glm::vec4(pos_) + glm::vec4(0.5f, 0.5f, -0.5f, 0.5f));
  corners.emplace_back(glm::vec4(pos_) + glm::vec4(0.5f, 0.5f, 0.5f, 0.5f));
  corners.emplace_back(glm::vec4(pos_) + glm::vec4(0.5f, 0.5f, 0.5f, -0.5f));
  for (int i = 0; i < 8; ++i) {
    std::vector<int> cube_indices;
    switch (i) {
      case 0: {
        std::vector<int> tmp_indices = {0, 1, 2, 3, 4, 5, 6, 7};
        cube_indices = tmp_indices;
        break;
      }
      case 1: {
        std::vector<int> tmp_indices = {8, 9, 10, 11, 12, 13, 14, 15};
        cube_indices = tmp_indices;
        break;
      }
      case 2: {
        std::vector<int> tmp_indices = {0, 1, 2, 3, 8, 9, 10, 11};
        cube_indices = tmp_indices;
        break;
      }
      case 3: {
        std::vector<int> tmp_indices = {4, 5, 6, 7, 12, 13, 14, 15};
        cube_indices = tmp_indices;
        break;
      }
      case 4: {
        std::vector<int> tmp_indices = {0, 1, 5, 4, 8, 9, 13, 12};
        cube_indices = tmp_indices;
        break;
      }
      case 5: {
        std::vector<int> tmp_indices = {3, 3, 6, 7, 11, 10, 14, 15};
        cube_indices = tmp_indices;
        break;
      }
      case 6: {
        std::vector<int> tmp_indices = {0, 4, 7, 3, 8, 12, 15, 11};
        cube_indices = tmp_indices;
        break;
      }
      case 7: {
        std::vector<int> tmp_indices = {1, 5, 6, 2, 9, 13, 14, 10};
        cube_indices = tmp_indices;
        break;
      }
    }
    std::vector<Tetrahedron> cube_tets = TesselateCube(corners, cube_indices);
    tets.insert(tets.end(), cube_tets.begin(), cube_tets.end());
  }
  return tets;
}

std::vector<Tetrahedron> Terrain::Block::TesselateCube(
    std::vector<glm::vec4>& corners, std::vector<int>& indices) {
  std::vector<Tetrahedron> vals;
  vals.emplace_back(corners[indices[0]], corners[indices[1]],
                    corners[indices[4]], corners[indices[3]]);
  vals.emplace_back(corners[indices[1]], corners[indices[2]],
                    corners[indices[3]], corners[indices[4]]);
  vals.emplace_back(corners[indices[2]], corners[indices[3]],
                    corners[indices[4]], corners[indices[7]]);
  vals.emplace_back(corners[indices[1]], corners[indices[2]],
                    corners[indices[4]], corners[indices[5]]);
  vals.emplace_back(corners[indices[2]], corners[indices[5]],
                    corners[indices[6]], corners[indices[7]]);
  return vals;
}
