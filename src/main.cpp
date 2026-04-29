// add header file
#include "mainLoop.hpp"

#include <cstdlib>
#include <iostream>
#include <stdexcept>

int main() {
  miniEngine::makeApp app{};

  try {
    app.run();

  } catch (const std::exception &e) {
    std::cerr << e.what() << "\n";
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}