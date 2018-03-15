#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <random>
#include <vector>

constexpr std::string_view
    usage("\n"
          "Name:\n"
          "\n"
          "tester - runs the lndir test suite\n"
          "\n"
          "Usage:\n"
          "\n"
          "tester [options] [test-filename [test-filename]...]\n"
          "\n"
          "Description:\n"
          "\n"
          "The tester program will execute one or more testcases against the\n"
          "lndir program, where each testcase is defined in a text file in\n"
          "the directory in which the tester program is run.\n"
          "\n"
          "If no testcases filenames are provided, then all testcase files\n"
          "found in the directory in which tester is run are executed.\n"
          "\n"
          "Options:\n"
          "\n"
          "--help\n"
          "     Display this usage help.\n\n");

namespace fs = std::filesystem;

enum class Path_type { dir, file, link };
enum class Test_status : bool { failed = false, passed = true };

struct Test_parms {
  std::vector<fs::path> testcase_filenames;
  bool want_help;
};

using Test_path = std::pair<Path_type, fs::path>;

struct Test_case {
  std::vector<Test_path> from_directories;
  fs::path lndir_suffix;
};

using Test_suite = std::map<std::string, Test_case>;

bool create_dir_tree(const fs::path &tree_root,
                     const std::vector<Test_path> &paths) {

  if (fs::exists(tree_root)) {
    std::cout << "\nERROR: <from-dir> exists: " << tree_root << "\n";
    return false;
  }

  std::error_code ec;

  for (const Test_path &path : paths) {
    switch (path.first) {
    case Path_type::dir:
      fs::create_directories(tree_root / path.second, ec);
      break;
    case Path_type::file:
      fs::create_directories(tree_root / path.second.parent_path(), ec);
      std::ofstream(tree_root / path.second);
      break;
    case Path_type::link:
      // not implemented
      break;
    }
  }

  return true;
}

bool call_lndir(fs::path from_tree_root, fs::path to_tree_root,
                fs::path suffix) noexcept {

  std::string command = "lndir";

  if (!suffix.empty()) {
    command += " --suffix " + suffix.string();
  }

  command += " " + from_tree_root.string() + " " + to_tree_root.string();

  int rc = std::system(command.c_str());
  if (rc != 0) {
    std::cout << "\nERROR: \"" << command << "\" failed with rc=" << rc << "\n";
    return false;
  }

  return true;
}

using Dir_tree_map = std::map<fs::path, Path_type>;

Dir_tree_map map_dir_tree(fs::path tree_root, std::string prefix,
                          std::string suffix = "") {

  std::error_code ec;
  fs::path path;
  Path_type path_type;
  Dir_tree_map dir_tree;

  fs::create_symlink(tree_root, prefix, ec);

  for (auto &dir_entry : fs::recursive_directory_iterator(prefix)) {
    path_type = dir_entry.is_directory() ? Path_type::dir : Path_type::file;
    path = dir_entry.path();

    if (!suffix.empty() && dir_entry.is_symlink()) {
      std::string::size_type pos = path.stem().string().rfind(suffix);
      if (pos != std::string::npos) {
        path = path.parent_path() / path.stem().string().substr(0, pos);
        path += dir_entry.path().extension();
      } else {
        path.clear();
      }
    }

    if (!path.empty()) {
      dir_tree.insert(std::make_pair(path, path_type));
    }
  }

  fs::remove(prefix, ec);

  return dir_tree;
}

bool compare_dir_trees(fs::path from_tree_root, fs::path to_tree_root,
                       std::string prefix, std::string suffix) {

  Dir_tree_map from_tree = map_dir_tree(from_tree_root, prefix);
  Dir_tree_map to_tree = map_dir_tree(to_tree_root, prefix, suffix);

  return from_tree == to_tree;
}

Test_status run_test(const Test_case &test_case) {

  Test_status status = Test_status::passed;
  std::error_code ec;
  std::random_device r;
  std::string random_prefix = std::to_string(r());
  fs::path from_tree_root = random_prefix + "_from_dir";
  fs::path to_tree_root = random_prefix + "_to_dir";

  if (create_dir_tree(from_tree_root, test_case.from_directories)) {
    fs::create_directory(to_tree_root, ec);
    if (call_lndir(from_tree_root, to_tree_root, test_case.lndir_suffix)) {
      if (!compare_dir_trees(from_tree_root, to_tree_root, random_prefix,
                             test_case.lndir_suffix.string())) {
        std::cout << "\nERROR: <to-dir> tree != <from-dir> tree!\n";
        status = Test_status::failed;
      }
    } else {
      std::cout << "\nERROR: Could not run lndir command!\n";
      status = Test_status::failed;
    }
  } else {
    std::cout << "\nERROR: Could not create <from_dir> tree\n";
    status = Test_status::failed;
  }

  fs::remove_all(from_tree_root, ec);
  fs::remove_all(to_tree_root, ec);

  return status;
}

int run_test_suite(Test_suite test_suite) {

  for (auto &test_case : test_suite) {
    std::cout << "Test: \"" << test_case.first << "\", status=";
    if (run_test(test_case.second) == Test_status::passed) {
      std::cout << "passed\n";
    } else {
      std::cout << "failed\n";
      return -1;
    }
  }

  return 0;
}

std::optional<Test_parms> parse_args(int argc, char *argv[]) {

  Test_parms parms = Test_parms();

  for (int arg_num = 1; arg_num < argc; ++arg_num) {
    if (std::string_view arg{argv[arg_num]}; arg == "--help") {
      parms.want_help = true;
      break;
    }
    parms.testcase_filenames.push_back(argv[arg_num]);
  }
  return parms;
}

std::optional<Test_suite> load_test_suite(const Test_parms &test_parms) {

  std::vector<fs::path> filenames = test_parms.testcase_filenames;

  if (filenames.empty()) {
    for (auto &file : fs::directory_iterator(".")) {
      if (file.is_regular_file() && file.path().extension() == ".test") {
        filenames.push_back(file.path());
      }
    }
  } else {
    for (fs::path &filename : filenames) {
      if (!filename.has_extension()) {
        filename += ".test";
      } else if (filename.extension() != ".test") {
        std::cout << "\nERROR: Testcase file doesn't have \".test\" extension: "
                  << filename << "\n";
        return std::nullopt;
      }
    }
  }

  if (filenames.empty()) {
    std::cout << "\nERROR: No testcase files found\n";
    return std::nullopt;
  }

  Test_suite test_suite = Test_suite();

  for (fs::path filename : filenames) {
    std::ifstream file(filename);
    if (!file) {
      std::cout << "\nERROR: Could not open testcase file " << filename << "\n";
      return std::nullopt;
    }

    Test_case test_case;
    std::string read_chunk, field1;
    fs::path field2;
    int chunk_num = 0;

    for (; file >> read_chunk; ++chunk_num) {
      if (chunk_num % 2 == 0) {
        field1 = read_chunk;
      } else {
        field2 = read_chunk;
        if (field1 == "file:") {
          test_case.from_directories.push_back(
              std::make_pair(Path_type::file, field2));
        } else if (field1 == "dir:") {
          test_case.from_directories.push_back(
              std::make_pair(Path_type::dir, field2));
        } else if (field1 == "suffix:") {
          if (field2 != "\"\"") {
            test_case.lndir_suffix = field2;
          }
        } else {
          std::cout << "\nERROR: Invalid file format: " << filename << "\n";
          return std::nullopt;
        }
      }
    }
    if (!file.eof()) {
      std::cout << "\nERROR: Unable to read " << filename << "\n";
      return std::nullopt;
    }
    if (chunk_num % 2 != 0) {
      std::cout << "\nERROR: Incorrect file format: " << filename << "\n";
      return std::nullopt;
    }

    auto result = test_suite.insert(std::make_pair(filename.stem(), test_case));
    if (!result.second) {
      std::cout << "\nERROR: More than one testcase with same name: "
                << filename.stem() << "\n";
      return std::nullopt;
    }
  }
  return test_suite;
}

int main(int argc, char *argv[]) {

  std::optional<Test_parms> test_parms = parse_args(argc, argv);
  if (!test_parms) {
    std::cout << "\nERROR: Invalid command-line invocation.\n";
    std::cout << usage;
    return -1;
  }
  if (test_parms->want_help) {
    std::cout << usage;
    return 0;
  }

  std::optional<Test_suite> test_suite = load_test_suite(*test_parms);
  if (!test_suite) {
    std::cout << "\nERROR: Could not load the testcases from the filesystem\n";
    return -1;
  }

  return run_test_suite(*test_suite);
}
