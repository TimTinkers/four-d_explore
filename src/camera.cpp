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
      zNear_(0.1),
      zFar_(100),
      radius_(1) {}

void Camera::SetTerrain(std::vector<glm::vec4>& t) {
  terrain_.clear();
  //std::copy(t.begin(), t.end(), std::inserter(terrain_, terrain_.end()));
  for (glm::vec4 v : t) {
    terrain_.insert(glm::ivec4(v));
  }
}

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

#define SENSITIVITY 2

void Camera::RotateUp(float amount) {
  mat5 rot = mat5::rotate(1, 3, SENSITIVITY * amount);
  view_matrix_ = rot * view_matrix_;
}

void Camera::RotateDown(float amount) {
  mat5 rot = mat5::rotate(1, 3, SENSITIVITY * -amount);
  view_matrix_ = rot * view_matrix_;
}

void Camera::RotateRight(float amount) {
  mat5 rot = mat5::rotate(0, 3, SENSITIVITY * -amount);
  view_matrix_ = rot * view_matrix_;
}

void Camera::RotateLeft(float amount) {
  mat5 rot = mat5::rotate(0, 3, SENSITIVITY * amount);
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

void Camera::RollLeft(float amount) {
  mat5 rot = mat5::rotate(0, 1, amount);
  view_matrix_ = rot * view_matrix_;
}

void Camera::RollRight(float amount) {
  mat5 rot = mat5::rotate(0, 1, -amount);
  view_matrix_ = rot * view_matrix_;
}

void Camera::MoveForward(float amount) {
  mat5 trans = mat5::translate(3, -amount);
  view_matrix_ = trans * view_matrix_;
  CheckCollision();
}

void Camera::MoveBackward(float amount) {
  mat5 trans = mat5::translate(3, amount);
  view_matrix_ = trans * view_matrix_;
  CheckCollision();
}

void Camera::MoveRight(float amount) {
  mat5 trans = mat5::translate(0, amount);
  view_matrix_ = trans * view_matrix_;
  CheckCollision();
}

void Camera::MoveLeft(float amount) {
  mat5 trans = mat5::translate(0, -amount);
  view_matrix_ = trans * view_matrix_;
  CheckCollision();
}

void Camera::MoveUp(float amount) {
  mat5 trans = mat5::translate(1, -amount);
  view_matrix_ = trans * view_matrix_;
  CheckCollision();
}

void Camera::MoveDown(float amount) {
  mat5 trans = mat5::translate(1, amount);
  view_matrix_ = trans * view_matrix_;
  CheckCollision();
}

void Camera::MoveAna(float amount) {
  mat5 trans = mat5::translate(2, amount);
  view_matrix_ = trans * view_matrix_;
  CheckCollision();
}

void Camera::MoveKata(float amount) {
  mat5 trans = mat5::translate(2, -amount);
  view_matrix_ = trans * view_matrix_;
  CheckCollision();
}

void PrintVec(const glm::vec4& v) {
  std::cout << "(" << v[0] << ", " << v[1] << ", " << v[2] << ", " << v[3]
            << ")\n";
}

void Camera::CheckCollision() {
  glm::vec4 col = view_matrix_.get_column();
  glm::mat4 rot = view_matrix_.get_main_mat();
  glm::vec4 c = -col*rot;

  glm::vec4 e;
  glm::vec4 closest_cell;
  //PrintVec(c);
  //std::cout << "\n";

  e = c + glm::vec4( radius_, 0, 0, 0);
  closest_cell = glm::ivec4(glm::round(e));
  if (terrain_.count(closest_cell) > 0) {
    float amount = 0.5 + (e - closest_cell)[0];
    mat5 trans = mat5::translate(0, amount);
    view_matrix_ = view_matrix_*trans;
  }

  e = c + glm::vec4(-radius_, 0, 0, 0);
  closest_cell = glm::ivec4(glm::round(e));
  if (terrain_.count(closest_cell) > 0) {
    float amount = 0.5 - (e - closest_cell)[0];
    mat5 trans = mat5::translate(0, -amount);
    view_matrix_ = view_matrix_*trans;
  }

  e = c + glm::vec4(0, radius_, 0, 0);
  closest_cell = glm::ivec4(glm::round(e));
  if (terrain_.count(closest_cell) > 0) {
    float amount = 0.5 + (e - closest_cell)[1];
    mat5 trans = mat5::translate(1, amount);
    view_matrix_ = view_matrix_*trans;
  }

  e = c + glm::vec4(0, -radius_, 0, 0);
  closest_cell = glm::ivec4(glm::round(e));
  if (terrain_.count(closest_cell) > 0) {
    float amount = 0.5 - (e - closest_cell)[1];
    mat5 trans = mat5::translate(1, -amount);
    view_matrix_ = view_matrix_*trans;
  }

  e = c + glm::vec4(0, 0, radius_, 0);
  closest_cell = glm::ivec4(glm::round(e));
  if (terrain_.count(closest_cell) > 0) {
    float amount = 0.5 + (e - closest_cell)[2];
    mat5 trans = mat5::translate(2, amount);
    view_matrix_ = view_matrix_*trans;
  }

  e = c + glm::vec4(0, 0, -radius_, 0);
  closest_cell = glm::ivec4(glm::round(e));
  if (terrain_.count(closest_cell) > 0) {
    float amount = 0.5 - (e - closest_cell)[2];
    mat5 trans = mat5::translate(2, -amount);
    view_matrix_ = view_matrix_*trans;
  }

  e = c + glm::vec4(0, 0, 0, radius_);
  closest_cell = glm::ivec4(glm::round(e));
  if (terrain_.count(closest_cell) > 0) {
    float amount = 0.5 + (e - closest_cell)[3];
    mat5 trans = mat5::translate(3, amount);
    view_matrix_ = view_matrix_ * trans;
  }

  e = c + glm::vec4(0, 0, 0, -radius_);
  closest_cell = glm::ivec4(glm::round(e));
  if (terrain_.count(closest_cell) > 0) {
    float amount = 0.5 - (e - closest_cell)[3];
    mat5 trans = mat5::translate(3, -amount);
    view_matrix_ = view_matrix_ * trans;
  }
}

mat5 Camera::GetViewProj() {
  return projection_matrix_ * view_matrix_;
}
