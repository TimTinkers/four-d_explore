#include "camera.h"

#include "matrix.h"

Camera::Camera()
    : eye_(0, 0, 0, -5),
      look_(0, 0, 0, 0),
      up_(0, 1, 0, 0),
      right_(1, 0, 0, 0),
      fovy_(30),
      aspectX_(1.77),
      aspectW_(1),
      zNear_(1),
      zFar_(20) {}

void Camera::SetEye(glm::vec4 eye) { eye_ = eye; }

void Camera::SetLook(glm::vec4 look) { look_ = look; }

void Camera::SetUpDir(glm::vec4 up) { up_ = up; }

void Camera::SetRightDir(glm::vec4 right) { right_ = right; }

void Camera::UpdateView() {
  //rot_matrix_ =
  //    mat5::lookAt(glm::vec4(0, 0, 0, 0), glm::vec4(0, 0, 0, 1), up_, right_);
  //trans_matrix_ = mat5::translate(3, -5);
  //rot_matrix_ =
  //    mat5::lookAt(eye_, look_, up_, right_);
  view_matrix_ = mat5::lookAt(eye_, look_, up_, right_);
}

void Camera::SetFovy(float fovy) { fovy_ = fovy; }

void Camera::SetAspectX(float aspectX) { aspectX_ = aspectX; }

void Camera::SetAspectW(float aspectW) { aspectW_ = aspectW; }

void Camera::SetZNear(float zNear) { zNear_ = zNear; }

void Camera::SetZFar(float zFar) { zFar_ = zFar; }

void Camera::UpdateProj() {
  projection_matrix_ =
      mat5::perspective(fovy_, aspectX_, aspectW_, zNear_, zFar_);
}

void Camera::RotateUp(float amount) {
  mat5 rot = mat5::rotate(1, 3, amount);
  view_matrix_ = rot * view_matrix_;
}

void Camera::RotateDown(float amount) {
  mat5 rot = mat5::rotate(1, 3, -amount);
  view_matrix_ = rot * view_matrix_;
}

void Camera::RotateRight(float amount) {
  mat5 rot = mat5::rotate(0, 3, -amount);
  view_matrix_ = rot * view_matrix_;
}

void Camera::RotateLeft(float amount) {
  mat5 rot = mat5::rotate(0, 3, amount);
  view_matrix_ = rot * view_matrix_;
}

void Camera::RotateAna(float amount) {
  mat5 rot = mat5::rotate(2, 3, amount);
  view_matrix_ = rot * view_matrix_;
}

void Camera::RotateKata(float amount) {
  mat5 rot = mat5::rotate(2, 3, -amount);
  view_matrix_ = rot * view_matrix_;
}

void Camera::MoveForward(float amount) {
  mat5 trans = mat5::translate(3, -amount);
  view_matrix_ = trans * view_matrix_;
}

void Camera::MoveBackward(float amount) {
  mat5 trans = mat5::translate(3, amount);
  view_matrix_ = trans * view_matrix_;
}

void Camera::MoveRight(float amount) {
  mat5 trans = mat5::translate(0, amount);
  view_matrix_ = trans * view_matrix_;
}

void Camera::MoveLeft(float amount) {
  mat5 trans = mat5::translate(0, -amount);
  view_matrix_ = trans * view_matrix_;
}

void Camera::MoveUp(float amount) {
  mat5 trans = mat5::translate(1, -amount);
  view_matrix_ = trans * view_matrix_;
}

void Camera::MoveDown(float amount) {
  mat5 trans = mat5::translate(1, amount);
  view_matrix_ = trans * view_matrix_;
}

void Camera::MoveAna(float amount) {
  mat5 trans = mat5::translate(2, amount);
  view_matrix_ = trans * view_matrix_;
}

void Camera::MoveKata(float amount) {
  mat5 trans = mat5::translate(2, -amount);
  view_matrix_ = trans * view_matrix_;
}

mat5 Camera::GetViewProj() {
  return projection_matrix_ * view_matrix_;//rot_matrix_ * trans_matrix_;
}
