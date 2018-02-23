#include <filesystem>

int main() {
  auto p = std::filesystem::current_path();
  return 0;
}
