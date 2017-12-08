#ifndef CAMERA_H_
#define CAMERA_H_

#include "matrix.h"

#include <iostream>

#include "glm/glm.hpp"

class Camera {
 public:
  Camera();

  // Only use following functions for setup. All functions must be called for
  // proper use.
  void SetEye(glm::vec4 eye);
  void SetLook(glm::vec4 look);
  void SetUpDir(glm::vec4 up);
  void SetRightDir(glm::vec4 right);

  void UpdateView();

  void SetFovy(float fovy);
  void SetAspectX(float aspectX);
  void SetAspectW(float aspectW);
  void SetZNear(float zNear);
  void SetZFar(float zFar);

  void UpdateProj();

  // These functions can be used to update current position.
  void RotateUp(float amount);
  void RotateDown(float amount);
  void RotateRight(float amount);
  void RotateLeft(float amount);
  void RotateAna(float amount);
  void RotateKata(float amount);

  void MoveForward(float amount);
  void MoveBackward(float amount);
  void MoveRight(float amount);
  void MoveLeft(float amount);
  void MoveUp(float amount);
  void MoveDown(float amount);
  void MoveAna(float amount);
  void MoveKata(float amount);

  mat5 getView() {
    std::cout << "trans:\n";
    trans_matrix_.Print();
    std::cout << "rot:\n";
    rot_matrix_.Print();
    return rot_matrix_ * trans_matrix_;
  }
  mat5 getProj() {
    return projection_matrix_;
  }
  mat5 GetViewProj();

 private:
  mat5 trans_matrix_;
  mat5 rot_matrix_;
  //mat5 view_matrix_;
  mat5 projection_matrix_;

  glm::vec4 eye_;
  glm::vec4 look_;
  glm::vec4 up_;
  glm::vec4 right_;

  float fovy_;
  float aspectX_;
  float aspectW_;
  float zNear_;
  float zFar_;
};

#endif  // CAMERA_H_
