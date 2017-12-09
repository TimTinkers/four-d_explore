#include "callback.h"

#include <iostream>

#define MOUSE_SCALE 0.001
#define SCROLL_SCALE 0.1

Callback* Callback::GetInstance() {
  static Callback instance;
  return &instance;
}

void Callback::init(App* app, Camera* camera, GLFWwindow* window) {
  app_ = app;
  camera_ = camera;
  scroll_pos_ = 0;
  glfwGetCursorPos(window, &last_x_pos_, &last_y_pos_);
  is_paused_ = false;
}

void Callback::on_keypress_event_impl(GLFWwindow* window, int key, int scanCode,
                                      int action, int mods) {
  //std::cout << (char)key << " " << action << "\n";
  if (action == GLFW_PRESS) {
    if (key == GLFW_KEY_ESCAPE) {
      glfwSetWindowShouldClose(window, true);
      keys_.clear();
      return;
    }
    if (key == GLFW_KEY_P) {
      keys_.clear();
      is_paused_ = !is_paused_;
      if (is_paused_) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
      } else {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        glfwGetCursorPos(window, &last_x_pos_, &last_y_pos_);
      }
    }
    if (!is_paused_) {
      keys_.insert(key);
    }
  } else if (action == GLFW_RELEASE) {
    if (keys_.count(key) == 1) {
      keys_.erase(keys_.find(key));
    }
  }
}

void Callback::on_mouse_button_event_impl(GLFWwindow* window, int button,
                                          int action, int mods) {
  //std::cout << "Mouse: " << button << "\n";
  //if (button == GLFW_MOUSE_BUTTON_1) {
  //  if (action == GLFW_PRESS) {
  //    mouse_down_ = true;
  //  } else if (action == GLFW_RELEASE) {
  //    mouse_down_ = false;
  //  }
  //}
}

void Callback::on_mouse_move_event_impl(GLFWwindow* window, double xPos,
                                        double yPos) {
  //std::cout << "MPos : (" << xPos << ", " << yPos << ")\n";
  //std::cout << "(" << last_x_pos_ << ", " << last_y_pos_ << ")\n";
  if (!is_paused_) {
    if (xPos < last_x_pos_) {
      camera_->RotateLeft(MOUSE_SCALE * (last_x_pos_ - xPos));
    } else if (xPos > last_x_pos_) {
      camera_->RotateRight(MOUSE_SCALE * (xPos - last_x_pos_));
    }
    if (yPos < last_y_pos_) {
      camera_->RotateUp(MOUSE_SCALE * (last_y_pos_ - yPos));
    } else if (yPos > last_y_pos_) {
      camera_->RotateDown(MOUSE_SCALE * (yPos - last_y_pos_));
    }

    last_x_pos_ = xPos;
    last_y_pos_ = yPos;
  }
}

void Callback::on_mouse_scroll_event_impl(GLFWwindow* window, double xOffset,
                                          double yOffset) {
  //std::cout << yOffset << "\n";
  scroll_pos_ += yOffset;
  camera_->RotateAna(SCROLL_SCALE * yOffset);
}
