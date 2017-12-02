#ifndef CALLBACK_H_
#define CALLBACK_H_

#include "app.h"

#include <unordered_set>

#include "GLFW/glfw3.h"

class Callback {
 public:
  static Callback* GetInstance();

  void init(App* app);

  const std::unordered_set<int>* get_keys() { return &keys_; }

  static void on_keypress_event(GLFWwindow* window, int key, int scanCode,
                                int action, int mods) {
    GetInstance()->on_keypress_event_impl(window, key, scanCode, action, mods);
  }

  static void on_mouse_button_event(GLFWwindow* window, int button, int action,
                                    int mods) {
    GetInstance()->on_mouse_button_event_impl(window, button, action, mods);
  }

  static void on_mouse_move_event(GLFWwindow* window, double xPos,
                                  double yPos) {
    GetInstance()->on_mouse_move_event_impl(window, xPos, yPos);
  }

  static void on_mouse_scroll_event(GLFWwindow* window, double xOffset,
                                    double yOffset) {
    GetInstance()->on_mouse_scroll_event_impl(window, xOffset, yOffset);
  }

 private:
  void on_keypress_event_impl(GLFWwindow* window, int key, int scanCode,
                              int action, int mods);
  void on_mouse_button_event_impl(GLFWwindow* window, int button, int action,
                                  int mods);
  void on_mouse_move_event_impl(GLFWwindow* window, double xPos, double yPos);

  void on_mouse_scroll_event_impl(GLFWwindow* window, double xOffset,
                                  double yOffset);

  App* app_;
  std::unordered_set<int> keys_;
};

#endif  // CALLBACK_H_
