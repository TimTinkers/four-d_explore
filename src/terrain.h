#ifndef TERRAIN_H_
#define TERRAIN_H_

// Imports.
#include <functional>
#include <tuple>
#include <unordered_map>
#include <vector>
#include "glm/glm.hpp"
#include "tetrahedron.h"

struct hashVec {
	size_t operator()(const glm::ivec4& c) const {
		size_t hash_x = std::hash<int>{}(c[0]);
		size_t hash_y = std::hash<int>{}(c[1]);
		size_t hash_z = std::hash<int>{}(c[2]);
		size_t hash_w = std::hash<int>{}(c[3]);
		hash_x ^= hash_y + 0x9e3779b97f4a7c16 + (hash_x << 6) + (hash_x >> 2);
		hash_x ^= hash_z + 0x9e3779b97f4a7c16 + (hash_x << 6) + (hash_x >> 2);
		hash_x ^= hash_w + 0x9e3779b97f4a7c16 + (hash_x << 6) + (hash_x >> 2);
		return hash_x;
	}
};

class Terrain {
public:
	class Block {
	public:
		Block(glm::ivec4 c, int t);
		glm::ivec4 GetPos();
		int GetType();

	private:
		glm::ivec4 pos_;
		int type_;
	};

	class Chunk {
	public:
		Chunk(glm::ivec4 c, float persistance, float frequency);
		Block* GetBlock(glm::ivec4 c);
		std::vector<Block*> GetAllBlocks();

	private:
		glm::ivec4 dimensions_;
		std::unordered_map<glm::ivec4, Block, hashVec> blocks_;
	};
};

#endif  // TERRAIN_H_
