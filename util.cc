#include "util.h"

#include <filesystem>
#include <iostream>
#include <string>
#include <system_error>

namespace utilities {

bool CreateDir(const std::string& path) {
  namespace fs = std::filesystem;
  try {
    // Extract the directory portion of the path.
    fs::path dir_path = fs::path(path).parent_path();

    // If a directory path exists and doesn't already exist, create it.
    if (!dir_path.empty() && !fs::exists(dir_path)) {
      std::error_code ec;
      fs::create_directories(dir_path, ec);
      if (ec) {
        // Directory creation failed.
        return false;
      }
    }
  } catch (const std::exception& e) {
    // Print the caught exception
    std::cerr << "Exception while saving image to '" << path
              << "': " << e.what() << std::endl;
    return false;
  }
  return true;
}

}  // namespace utilities
