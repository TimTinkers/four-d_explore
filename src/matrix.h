// This file defines a 5x5 matrix.
// The matrix is arranged as such generally:
// [x y 'w' z w]
//
// The w values are used to for the fourth dimension (ana and kata).
// It is inserted between y and z because is is analogous to x and y in our math

#ifndef MATRIX_H_
#define MATRIX_H_

#include "glm/glm.hpp"

class mat5;

glm::vec4 cross4(glm::vec4 U, glm::vec4 V, glm::vec4 W);

class vec5 {
 public:
  vec5();

  friend class mat5;

 private:
  glm::vec4 vec;
  float w;
};

class mat5 {
 public:
  mat5();

  vec5 operator*(const vec5& other);

  mat5 operator*(const mat5& other);

  static mat5 perspective(float fovy, float aspectx, float aspectw, float zNear,
                          float zFar);

  static mat5 lookAt(glm::vec4 eye, glm::vec4 center, glm::vec4 up,
                     glm::vec4 right);

  // Generate a rotation matrix between axisA and axisB.
  static mat5 rotate(int axisA, int axisB, float angle);

  // Gererate a translation matrix along a given axis.
  static mat5 translate(int axis, float amount);

  void Print() const;

 private:
  glm::mat4 main_mat;
  glm::vec4 column;
  glm::vec4 row;
  float ww;
};

#endif  // MATRIX_H_
