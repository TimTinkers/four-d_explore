#include "app.h"

#include <memory>

int main() {
  std::shared_ptr<App> app_ptr(new App());
  app_ptr->init();
  app_ptr->run();
  return 0;
}
