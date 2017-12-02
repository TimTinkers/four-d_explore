#include "callback.h"

#include <iostream>

Callback* Callback::GetInstance() {
  static Callback instance;
  return &instance;
}

void Callback::init(App* app) {
  app_ = app;
}

void Callback::on_keypress_event_impl(GLFWwindow* window, int key, int scanCode,
                                      int action, int mods) {
  std::cout << (char)key << " " << action << "\n";
  if (action == GLFW_PRESS) {
    keys_.insert(key);
  } else if (action == GLFW_RELEASE) {
    if (keys_.count(key) == 1) {
      keys_.erase(keys_.find(key));
    }
  }
}

void Callback::on_mouse_button_event_impl(GLFWwindow* window, int button,
                                          int action, int mods) {
  std::cout << "Mouse: " << button << "\n";
}

void Callback::on_mouse_move_event_impl(GLFWwindow* window, double xPos,
                                        double yPos) {
  //std::cout << "MPos : (" << xPos << ", " << yPos << ")\n";
}

void Callback::on_mouse_scroll_event_impl(GLFWwindow* window, double xOffset,
                                          double yOffset) {
  std::cout << yOffset << "\n";
}
