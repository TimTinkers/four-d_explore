#include "matrix.h"

#include "glm/gtc/matrix_access.hpp"

#include <iostream>

// This algorithm taken from Steven Hollasch's Master's Thesis on four-space
// visualization.
// It extends the cross product to four dimensions using determinants to
// generate a mutually perpendicular vector given 3 input vectors.
glm::vec4 cross4(glm::vec4 U, glm::vec4 V, glm::vec4 W) {
  glm::vec4 result;
  float A, B, C, D, E, F;

  A = (V[0] * W[1]) - (V[1] * W[0]);
  B = (V[0] * W[2]) - (V[2] * W[0]);
  C = (V[0] * W[3]) - (V[3] * W[0]);
  D = (V[1] * W[2]) - (V[2] * W[1]);
  E = (V[1] * W[3]) - (V[3] * W[1]);
  F = (V[2] * W[3]) - (V[3] * W[2]);

  result[0] = (U[1] * F) - (U[2] * E) + (U[3] * D);
  result[1] = (U[0] * F) - (U[2] * C) + (U[3] * B);
  result[2] = (U[0] * E) - (U[1] * C) + (U[3] * A);
  result[3] = (U[0] * D) - (U[1] * B) + (U[2] * A);

  return result;
}

vec5::vec5() {
  vec = glm::vec4(0);
  w = 0;
}

mat5::mat5() {
  main_mat = glm::mat4(1);
  column = glm::vec4(0);
  row = glm::vec4(0);
  ww = 1;
}

vec5 mat5::operator*(const vec5& other) {
  vec5 result;
  result.vec = main_mat * other.vec + column * glm::vec4(other.w);
  result.w = glm::dot(row, glm::vec4(other.w)) + ww * other.w;
  return result;
}

mat5 mat5::operator*(const mat5& other) {
  mat5 result;
  //other.Print();
  result.main_mat = main_mat * other.main_mat;
  for (int i = 0; i < 4; ++i) {
    for (int j = 0; j < 4; ++j) {
      result.main_mat[i][j] += column[i] * other.row[j];
    }
  }
  for (int i = 0; i < 4; ++i) {
    result.column[i] =
        glm::dot(main_mat[i], other.column) + column[i] * other.ww;
    result.row[i] =
        glm::dot(row, glm::column(other.main_mat, i)) + ww * other.row[i];
  }
  result.ww = glm::dot(row, other.column) + ww * other.ww;
  return result;
}

mat5 mat5::perspective(float fovy, float aspectx, float aspectw,
                              float zNear, float zFar) {
  mat5 result;
  float const tanHalfFovy = tan(fovy / 2.0f);

  result.main_mat[0][0] = 1 / (aspectx * tanHalfFovy);
  result.main_mat[1][1] = 1 / (tanHalfFovy);
  result.main_mat[2][2] = 1 / (aspectw * tanHalfFovy);
  result.main_mat[3][3] = zFar / (zNear - zFar);
  result.row[3] = -1;
  result.column[3] = -(zFar * zNear) / (zFar - zNear);
  return result;
}

mat5 mat5::lookAt(glm::vec4 eye, glm::vec4 center, glm::vec4 up,
                  glm::vec4 right) {
  mat5 result;

  const glm::vec4 f(normalize(center - eye)); // front
  const glm::vec4 a(normalize(cross4(f, up, right))); // ana
  const glm::vec4 r(normalize(cross4(f, up, a))); // right
  const glm::vec4 u(normalize(cross4(f, a, r))); // up

  result.main_mat[0][0] = r.x;
  result.main_mat[1][0] = r.y;
  result.main_mat[2][0] = r.z;
  result.main_mat[3][0] = r.w;
  result.main_mat[0][1] = u.x;
  result.main_mat[1][1] = u.y;
  result.main_mat[2][1] = u.z;
  result.main_mat[3][1] = u.w;
  result.main_mat[0][2] = a.x;
  result.main_mat[1][2] = a.y;
  result.main_mat[2][2] = a.z;
  result.main_mat[3][2] = a.w;
  result.main_mat[0][3] =-f.x;
  result.main_mat[1][3] =-f.y;
  result.main_mat[2][3] =-f.z;
  result.main_mat[3][3] =-f.w;
  result.column[0] = -dot(r, eye);
  result.column[1] = -dot(u, eye);
  result.column[2] = -dot(a, eye);
  result.column[3] =  dot(f, eye);

  return result;
}

mat5 mat5::rotate(int axisA, int axisB, float angle) {
  mat5 res;
  res.main_mat[0][0] = 1.0f;
  res.main_mat[1][1] = 1.0f;
  res.main_mat[2][2] = 1.0f;
  res.main_mat[3][3] = 1.0f;
  res.ww = 1.0f;

  float cosA = cos(angle);
  float sinA = sin(angle);
  res.main_mat[axisA][axisA] = cosA;
  res.main_mat[axisA][axisB] = sinA;
  res.main_mat[axisB][axisA] = -sinA;
  res.main_mat[axisB][axisB] = cosA;
  return res;
}

mat5 mat5::translate(int axis, float amount) {
  mat5 res;
  res.main_mat[0][0] = 1.0f;
  res.main_mat[1][1] = 1.0f;
  res.main_mat[2][2] = 1.0f;
  res.main_mat[3][3] = 1.0f;
  res.ww = 1.0f;
  
  res.column[axis] = amount;
  return res;
}

void mat5::Print() const {
  std::cout << "[" << main_mat[0][0] << ", " << main_mat[0][1] << ", "
            << main_mat[0][2] << ", " << main_mat[0][3] << ", " << column[0]
            << "\n";
  std::cout << " " << main_mat[1][0] << ", " << main_mat[1][1] << ", "
            << main_mat[1][2] << ", " << main_mat[1][3] << ", " << column[1]
            << "\n";
  std::cout << " " << main_mat[2][0] << ", " << main_mat[2][1] << ", "
            << main_mat[2][2] << ", " << main_mat[2][3] << ", " << column[2]
            << "\n";
  std::cout << " " << main_mat[3][0] << ", " << main_mat[3][1] << ", "
            << main_mat[3][2] << ", " << main_mat[3][3] << ", " << column[3]
            << "\n";
  std::cout << " " << row[0] << ", " << row[1] << ", " << row[2] << ", "
            << row[3] << ", " << ww << "]\n";
}
