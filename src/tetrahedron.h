#ifndef TETRAHEDRON_H_
#define TETRAHEDRON_H_

// Imports.
#include "glm/glm.hpp"
#include <vector>

class Tetrahedron {
	public:
		Tetrahedron(glm::vec4 a, glm::vec4 b, glm::vec4 c, glm::vec4 d) {
			coords_.push_back(a);
			coords_.push_back(b);
			coords_.push_back(c);
			coords_.push_back(d);
		}

	private:
		std::vector<glm::vec4> coords_;
};

#endif  // TETRAHEDRON_H_
